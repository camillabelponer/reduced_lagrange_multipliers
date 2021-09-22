/* ---------------------------------------------------------------------
 */
#ifndef dealii_distributed_lagrange_multiplier_h
#define dealii_distributed_lagrange_multiplier_h

#include <deal.II/base/function.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/timer.h>

#include <deal.II/lac/block_linear_operator.h>
#include <deal.II/lac/generic_linear_algebra.h>
#include <deal.II/lac/linear_operator.h>
#include <deal.II/lac/linear_operator_tools.h>
#define FORCE_USE_OF_TRILINOS
namespace LA
{
#if defined(DEAL_II_WITH_PETSC) && !defined(DEAL_II_PETSC_WITH_COMPLEX) && \
  !(defined(DEAL_II_WITH_TRILINOS) && defined(FORCE_USE_OF_TRILINOS))
  using namespace dealii::LinearAlgebraPETSc;
#  define USE_PETSC_LA
#elif defined(DEAL_II_WITH_TRILINOS)
  using namespace dealii::LinearAlgebraTrilinos;
#else
#  error DEAL_II_WITH_PETSC or DEAL_II_WITH_TRILINOS required
#endif
} // namespace LA
#include <deal.II/base/conditional_ostream.h>
#include <deal.II/base/index_set.h>
#include <deal.II/base/parameter_acceptor.h>
#include <deal.II/base/parsed_function.h>
#include <deal.II/base/utilities.h>

#include <deal.II/distributed/grid_refinement.h>
#include <deal.II/distributed/solution_transfer.h>
#include <deal.II/distributed/tria.h>

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_nothing.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/mapping_fe_field.h>
#include <deal.II/fe/mapping_q.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/manifold_lib.h>

#include <deal.II/lac/affine_constraints.h>
#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/petsc_precondition.h>
#include <deal.II/lac/petsc_solver.h>
#include <deal.II/lac/petsc_sparse_matrix.h>
#include <deal.II/lac/petsc_vector.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/solver_gmres.h>
#include <deal.II/lac/solver_minres.h>
#include <deal.II/lac/sparsity_tools.h>
#include <deal.II/lac/vector.h>

#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/numerics/vector_tools.h>

#include <deal.II/opencascade/manifold_lib.h>
#include <deal.II/opencascade/utilities.h>

#include <deal.II/particles/data_out.h>
#include <deal.II/particles/generators.h>
#include <deal.II/particles/particle_handler.h>
#include <deal.II/particles/utilities.h>
#ifdef DEAL_II_WITH_OPENCASCADE
#  include <TopoDS.hxx>
#endif
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>

using namespace dealii;
template <int dim, int spacedim = dim>
class ProblemParameters : public ParameterAcceptor
{
public:
  ProblemParameters();

  std::string                   output_directory       = ".";
  unsigned int                  fe_degree              = 2;
  unsigned int                  initial_refinement     = 5;
  unsigned int                  rtree_extraction_level = 1;
  std::list<types::boundary_id> dirichlet_ids{0};
  std::string                   name_of_grid        = "hyper_cube";
  std::string                   arguments_for_grid  = "-1: 1: false";
  std::string                   refinement_strategy = "fixed_fraction";
  double                        coarsening_fraction = 0.3;
  double                        refinement_fraction = 0.3;
  unsigned int                  max_cells           = 20000;
  mutable ParameterAcceptorProxy<Functions::ParsedFunction<spacedim>> rhs;
  mutable ParameterAcceptorProxy<Functions::ParsedFunction<spacedim>> bc;

  mutable ParameterAcceptorProxy<Functions::ParsedFunction<spacedim>>
    inclusions_rhs;

  std::vector<std::vector<double>> inclusions             = {{-.2, -.2, .3}};
  unsigned int                     inclusions_refinement  = 1000;
  unsigned int                     n_fourier_coefficients = 1;
};



template <int dim, int spacedim>
ProblemParameters<dim, spacedim>::ProblemParameters()
  : ParameterAcceptor("Immersed Problem/")
  , rhs("Right hand side")
  , bc("Dirichlet boundary conditions")
  , inclusions_rhs("Immersed inclusions/Boundary data")
{
  add_parameter("FE degree", fe_degree, "", this->prm, Patterns::Integer(1));
  add_parameter("Output directory", output_directory);
  add_parameter("Initial refinement", initial_refinement);
  add_parameter("Bounding boxes extraction level", rtree_extraction_level);
  add_parameter("Dirichlet boundary ids", dirichlet_ids);
  enter_subsection("Grid generation");
  {
    add_parameter("Grid generator", name_of_grid);
    add_parameter("Grid generator arguments", arguments_for_grid);
  }
  leave_subsection();
  enter_subsection("Refinement and remeshing");
  {
    add_parameter("Strategy",
                  refinement_strategy,
                  "",
                  this->prm,
                  Patterns::Selection("fixed_fraction|fixed_number"));
    add_parameter("Coarsening fraction", coarsening_fraction);
    add_parameter("Refinement fraction", refinement_fraction);
    add_parameter("Maximum number of cells", max_cells);
  }
  leave_subsection();
  enter_subsection("Immersed inclusions");
  {
    add_parameter("Inclusions", inclusions);
    add_parameter("Inclusions refinement", inclusions_refinement);
    add_parameter("Number of fourier coefficients", n_fourier_coefficients);
  }
  leave_subsection();
}



template <int spacedim>
struct ReferenceInclusion
{
  ReferenceInclusion(unsigned int n_q_points, unsigned int n_coefficients)
    : n_q_points(n_q_points)
    , n_coefficients(n_coefficients)
    , support_points(n_q_points)
    , theta(n_q_points)
    , current_support_points(n_q_points)
    , current_fe_values(n_coefficients)
  {
    static_assert(spacedim > 1, "Not implemented in dim = 1");
    for (unsigned int i = 0; i < n_q_points; ++i)
      {
        theta[i]             = i * 2 * numbers::PI / n_q_points;
        support_points[i][0] = std::cos(theta[i]);
        support_points[i][1] = std::sin(theta[i]);
      }
  }


  std::vector<types::global_dof_index>
  get_dof_indices(const types::global_dof_index &id) const
  {
    std::vector<types::global_dof_index> dofs(n_coefficients);
    auto start_index = (id / n_q_points) * n_coefficients;
    for (auto &d : dofs)
      d = start_index++;
    return dofs;
  }


  const unsigned int           n_q_points;
  const unsigned int           n_coefficients;
  std::vector<Point<spacedim>> support_points;
  std::vector<double>          theta;

  // Current configuration
  unsigned int    current_inclusion_id = numbers::invalid_unsigned_int;
  double          current_radius;
  Point<spacedim> current_center;
  mutable std::vector<Point<spacedim>> current_support_points;
  std::vector<double>                  current_fe_values;

  const std::vector<double> &
  reinit(const types::global_dof_index           particle_id,
         const std::vector<std::vector<double>> &inclusions)
  {
    if (n_coefficients == 0)
      return current_fe_values;
    const auto q  = particle_id % n_q_points;
    const auto id = particle_id / n_q_points;
    AssertIndexRange(id, inclusions.size());
    AssertDimension(inclusions[id].size(), spacedim + 1);
    const auto r         = inclusions[id][spacedim];
    const auto ds        = 2 * numbers::PI * r / n_q_points;
    current_fe_values[0] = ds;
    for (unsigned int c = 1; c < n_coefficients; ++c)
      {
        unsigned int omega = (c + 1) / 2;
        const double rho   = std::pow(r, omega);
        if ((c + 1) % 2 == 0)
          current_fe_values[c] = ds * rho * std::cos(theta[q] * omega);
        else
          current_fe_values[c] = ds * rho * std::sin(theta[q] * omega);
      }
    return current_fe_values;
  }

  const std::vector<Point<spacedim>> &
  get_current_support_points(const std::vector<double> &inclusion) const
  {
    AssertDimension(inclusion.size(), spacedim + 1);
    Point<spacedim> center;
    for (unsigned int d = 0; d < spacedim; ++d)
      center[d] = inclusion[d];

    const auto &r = inclusion[spacedim];
    for (unsigned int q = 0; q < n_q_points; ++q)
      current_support_points[q] = center + support_points[q] * r;
    return current_support_points;
  }
};


template <int dim, int spacedim = dim>
class PoissonProblem : public Subscriptor
{
public:
  PoissonProblem(const ProblemParameters<dim, spacedim> &par);
  void
  make_grid();
  void
  setup_inclusions_particles();
  void
  setup_fe();
  void
  setup_dofs();
  void
  assemble_poisson_system();
  void
  assemble_coupling();
  void
  run();

  /**
   * Builds coupling sparsity, and returns locally relevant inclusion dofs.
   */
  IndexSet
  assemble_coupling_sparsity(DynamicSparsityPattern &dsp) const;

  void
  solve();
  void
  refine_and_transfer();
  void
  output_solution() const;

  void
  output_particles() const;

  void
  print_parameters() const;

  types::global_dof_index
  n_inclusions_dofs() const;

private:
  const ProblemParameters<dim, spacedim> &       par;
  MPI_Comm                                       mpi_communicator;
  ConditionalOStream                             pcout;
  mutable TimerOutput                            computing_timer;
  parallel::distributed::Triangulation<spacedim> tria;
  std::unique_ptr<FiniteElement<spacedim>>       fe;
  std::unique_ptr<ReferenceInclusion<spacedim>>  inclusion;
  std::unique_ptr<Quadrature<spacedim>>          quadrature;
  DoFHandler<spacedim>                           dh;
  std::vector<IndexSet>                          owned_dofs;
  std::vector<IndexSet>                          relevant_dofs;

  Particles::ParticleHandler<spacedim> inclusions_as_particles;

  AffineConstraints<double> constraints;
  AffineConstraints<double> inclusion_constraints;

  LA::MPI::SparseMatrix                           stiffness_matrix;
  LA::MPI::SparseMatrix                           coupling_matrix;
  LA::MPI::BlockVector                            solution;
  LA::MPI::BlockVector                            locally_relevant_solution;
  LA::MPI::BlockVector                            system_rhs;
  std::vector<std::vector<BoundingBox<spacedim>>> global_bounding_boxes;
};


template <int dim, int spacedim>
PoissonProblem<dim, spacedim>::PoissonProblem(
  const ProblemParameters<dim, spacedim> &par)
  : par(par)
  , mpi_communicator(MPI_COMM_WORLD)
  , pcout(std::cout, (Utilities::MPI::this_mpi_process(mpi_communicator) == 0))
  , computing_timer(mpi_communicator,
                    pcout,
                    TimerOutput::summary,
                    TimerOutput::wall_times)
  , tria(mpi_communicator,
         typename Triangulation<spacedim>::MeshSmoothing(
           Triangulation<spacedim>::smoothing_on_refinement |
           Triangulation<spacedim>::smoothing_on_coarsening))
  , dh(tria)
{}



template <int dim, int spacedim>
void
read_grid_and_cad_files(const std::string &           grid_file_name,
                        const std::string &           ids_and_cad_file_names,
                        Triangulation<dim, spacedim> &tria)
{
  GridIn<dim, spacedim> grid_in;
  grid_in.attach_triangulation(tria);
  grid_in.read(grid_file_name);
#ifdef DEAL_II_WITH_OPENCASCADE
  using map_type  = std::map<types::manifold_id, std::string>;
  using Converter = Patterns::Tools::Convert<map_type>;
  for (const auto &pair : Converter::to_value(ids_and_cad_file_names))
    {
      const auto &manifold_id   = pair.first;
      const auto &cad_file_name = pair.second;
      const auto  extension     = boost::algorithm::to_lower_copy(
        cad_file_name.substr(cad_file_name.find_last_of('.') + 1));
      TopoDS_Shape shape;
      if (extension == "iges" || extension == "igs")
        shape = OpenCASCADE::read_IGES(cad_file_name);
      else if (extension == "step" || extension == "stp")
        shape = OpenCASCADE::read_STEP(cad_file_name);
      else
        AssertThrow(false,
                    ExcNotImplemented("We found an extension that we "
                                      "do not recognize as a CAD file "
                                      "extension. Bailing out."));
      const auto n_elements = OpenCASCADE::count_elements(shape);
      if ((std::get<0>(n_elements) == 0))
        tria.set_manifold(
          manifold_id,
          OpenCASCADE::ArclengthProjectionLineManifold<dim, spacedim>(shape));
      else if (spacedim == 3)
        {
          const auto t = reinterpret_cast<Triangulation<dim, 3> *>(&tria);
          t->set_manifold(manifold_id,
                          OpenCASCADE::NormalToMeshProjectionManifold<dim, 3>(
                            shape));
        }
      else
        tria.set_manifold(manifold_id,
                          OpenCASCADE::NURBSPatchManifold<dim, spacedim>(
                            TopoDS::Face(shape)));
    }
#else
  (void)ids_and_cad_file_names;
  AssertThrow(false, ExcNotImplemented("Generation of the grid failed."));
#endif
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::make_grid()
{
  try
    {
      GridGenerator::generate_from_name_and_arguments(tria,
                                                      par.name_of_grid,
                                                      par.arguments_for_grid);
    }
  catch (...)
    {
      pcout << "Generating from name and argument failed." << std::endl
            << "Trying to read from file name." << std::endl;
      read_grid_and_cad_files(par.name_of_grid, par.arguments_for_grid, tria);
    }
  tria.refine_global(par.initial_refinement);
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::setup_inclusions_particles()
{
  if (par.inclusions.empty())
    return;

  inclusions_as_particles.initialize(tria, StaticMappingQ1<spacedim>::mapping);

  std::vector<Point<spacedim>> particles_positions;
  particles_positions.reserve(inclusion->n_q_points * par.inclusions.size());
  for (unsigned int i = 0; i < par.inclusions.size(); ++i)
    {
      const auto &p = inclusion->get_current_support_points(par.inclusions[i]);
      particles_positions.insert(particles_positions.end(), p.begin(), p.end());
    }

  std::vector<BoundingBox<spacedim>> all_boxes;
  all_boxes.reserve(tria.n_locally_owned_active_cells());
  for (const auto &cell : tria.active_cell_iterators())
    if (cell->is_locally_owned())
      all_boxes.emplace_back(cell->bounding_box());
  const auto tree = pack_rtree(all_boxes);
  const auto local_boxes =
    extract_rtree_level(tree, par.rtree_extraction_level);

  global_bounding_boxes =
    Utilities::MPI::all_gather(mpi_communicator, local_boxes);

  Assert(!global_bounding_boxes.empty(),
         ExcInternalError(
           "I was expecting the "
           "global_bounding_boxes to be filled at this stage. "
           "Make sure you fill this vector before trying to use it "
           "here. Bailing out."));
  inclusions_as_particles.insert_global_particles(particles_positions,
                                                  global_bounding_boxes);
  tria.signals.pre_distributed_refinement.connect(
    [&]() { inclusions_as_particles.register_store_callback_function(); });
  tria.signals.post_distributed_refinement.connect(
    [&]() { inclusions_as_particles.register_load_callback_function(false); });
  pcout << "Inclusions particles: "
        << inclusions_as_particles.n_global_particles() << std::endl;
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::setup_fe()
{
  TimerOutput::Scope t(computing_timer, "Initial setup");
  fe = std::make_unique<FESystem<spacedim>>(FE_Q<spacedim>(par.fe_degree), 1);
  quadrature = std::make_unique<QGauss<spacedim>>(par.fe_degree + 1);
  inclusion =
    std::make_unique<ReferenceInclusion<spacedim>>(par.inclusions_refinement,
                                                   par.n_fourier_coefficients);
}


template <int dim, int spacedim>
types::global_dof_index
PoissonProblem<dim, spacedim>::n_inclusions_dofs() const
{
  if (!par.inclusions.empty())
    return par.inclusions.size() * par.n_fourier_coefficients;
  else
    return 0;
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::setup_dofs()
{
  TimerOutput::Scope t(computing_timer, "Setup dofs");
  dh.distribute_dofs(*fe);

  owned_dofs.resize(2);
  owned_dofs[0] = dh.locally_owned_dofs();
  relevant_dofs.resize(2);
  DoFTools::extract_locally_relevant_dofs(dh, relevant_dofs[0]);
  {
    constraints.reinit(relevant_dofs[0]);
    DoFTools::make_hanging_node_constraints(dh, constraints);
    for (const auto id : par.dirichlet_ids)
      VectorTools::interpolate_boundary_values(dh, id, par.bc, constraints);
    constraints.close();
  }
  {
    stiffness_matrix.clear();
    DynamicSparsityPattern dsp(relevant_dofs[0]);
    DoFTools::make_sparsity_pattern(dh, dsp, constraints, false);
    SparsityTools::distribute_sparsity_pattern(dsp,
                                               owned_dofs[0],
                                               mpi_communicator,
                                               relevant_dofs[0]);
    stiffness_matrix.reinit(owned_dofs[0],
                            owned_dofs[0],
                            dsp,
                            mpi_communicator);
  }
  {
    auto inclusions_set =
      Utilities::MPI::create_evenly_distributed_partitioning(
        mpi_communicator, par.inclusions.size());

    owned_dofs[1] = inclusions_set.tensor_product(
      complete_index_set(par.n_fourier_coefficients));

    coupling_matrix.clear();
    DynamicSparsityPattern dsp(dh.n_dofs(), n_inclusions_dofs());

    relevant_dofs[1] = assemble_coupling_sparsity(dsp);
    SparsityTools::distribute_sparsity_pattern(dsp,
                                               owned_dofs[0],
                                               mpi_communicator,
                                               relevant_dofs[0]);
    coupling_matrix.reinit(owned_dofs[0], owned_dofs[1], dsp, mpi_communicator);
    inclusion_constraints.close();
  }

  locally_relevant_solution.reinit(owned_dofs, relevant_dofs, mpi_communicator);
  system_rhs.reinit(owned_dofs, mpi_communicator);
  solution.reinit(owned_dofs, mpi_communicator);

  pcout << "   Number of degrees of freedom: " << owned_dofs[0].size() << " + "
        << owned_dofs[1].size()
        << " (locally owned: " << owned_dofs[0].n_elements() << " + "
        << owned_dofs[1].n_elements() << ")" << std::endl;
}

template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::assemble_poisson_system()
{
  stiffness_matrix = 0;
  coupling_matrix  = 0;
  system_rhs       = 0;
  TimerOutput::Scope               t(computing_timer, "Assemble Stiffness");
  FEValues<spacedim>               fe_values(*fe,
                               *quadrature,
                               update_values | update_gradients |
                                 update_quadrature_points | update_JxW_values);
  const unsigned int               dofs_per_cell = fe->n_dofs_per_cell();
  const unsigned int               n_q_points    = quadrature->size();
  FullMatrix<double>               cell_matrix(dofs_per_cell, dofs_per_cell);
  Vector<double>                   cell_rhs(dofs_per_cell);
  std::vector<double>              rhs_values(n_q_points);
  std::vector<Tensor<1, spacedim>> grad_phi_u(dofs_per_cell);
  std::vector<types::global_dof_index> local_dof_indices(dofs_per_cell);
  const FEValuesExtractors::Scalar     scalar(0);
  for (const auto &cell : dh.active_cell_iterators())
    if (cell->is_locally_owned())
      {
        cell_matrix = 0;
        cell_rhs    = 0;
        fe_values.reinit(cell);
        par.rhs.value_list(fe_values.get_quadrature_points(), rhs_values);
        for (unsigned int q = 0; q < n_q_points; ++q)
          {
            for (unsigned int k = 0; k < dofs_per_cell; ++k)
              grad_phi_u[k] = fe_values[scalar].gradient(k, q);
            for (unsigned int i = 0; i < dofs_per_cell; ++i)
              {
                for (unsigned int j = 0; j < dofs_per_cell; ++j)
                  {
                    cell_matrix(i, j) +=
                      grad_phi_u[i] * grad_phi_u[j] * fe_values.JxW(q);
                  }
                cell_rhs(i) += fe_values.shape_value(i, q) * rhs_values[q] *
                               fe_values.JxW(q);
              }
          }
        cell->get_dof_indices(local_dof_indices);
        constraints.distribute_local_to_global(cell_matrix,
                                               cell_rhs,
                                               local_dof_indices,
                                               stiffness_matrix,
                                               system_rhs.block(0));
      }
  stiffness_matrix.compress(VectorOperation::add);
  system_rhs.compress(VectorOperation::add);
}



template <int dim, int spacedim>
IndexSet
PoissonProblem<dim, spacedim>::assemble_coupling_sparsity(
  DynamicSparsityPattern &dsp) const
{
  TimerOutput::Scope t(computing_timer, "Assemble Coupling sparsity");
  IndexSet           relevant(n_inclusions_dofs());

  const FEValuesExtractors::Scalar scalar(0);

  std::vector<types::global_dof_index> dof_indices(fe->n_dofs_per_cell());
  std::vector<types::global_dof_index> inclusion_dof_indices;

  auto particle = inclusions_as_particles.begin();
  while (particle != inclusions_as_particles.end())
    {
      const auto &cell = particle->get_surrounding_cell(tria);
      const auto &dh_cell =
        typename DoFHandler<spacedim>::cell_iterator(*cell, &dh);
      dh_cell->get_dof_indices(dof_indices);

      const auto pic = inclusions_as_particles.particles_in_cell(cell);
      Assert(pic.begin() == particle, ExcInternalError());
      std::set<types::global_dof_index> inclusion_dof_indices_set;
      for (const auto &p : pic)
        {
          const auto ids = inclusion->get_dof_indices(p.get_id());
          inclusion_dof_indices_set.insert(ids.begin(), ids.end());
        }
      inclusion_dof_indices.resize(0);
      inclusion_dof_indices.insert(inclusion_dof_indices.begin(),
                                   inclusion_dof_indices_set.begin(),
                                   inclusion_dof_indices_set.end());

      constraints.add_entries_local_to_global(dof_indices,
                                              inclusion_dof_indices,
                                              dsp);
      relevant.add_indices(inclusion_dof_indices.begin(),
                           inclusion_dof_indices.end());
      particle = pic.end();
    }
  return relevant;
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::assemble_coupling()
{
  TimerOutput::Scope t(computing_timer, "Assemble Coupling matrix");
  const FEValuesExtractors::Scalar     scalar(0);
  std::vector<types::global_dof_index> fe_dof_indices(fe->n_dofs_per_cell());
  std::vector<types::global_dof_index> inclusion_dof_indices(
    inclusion->n_coefficients);

  FullMatrix<double> local_matrix(fe->n_dofs_per_cell(),
                                  inclusion->n_coefficients);
  Vector<double>     local_rhs(inclusion->n_coefficients);

  auto particle = inclusions_as_particles.begin();
  while (particle != inclusions_as_particles.end())
    {
      const auto &cell = particle->get_surrounding_cell(tria);
      const auto &dh_cell =
        typename DoFHandler<spacedim>::cell_iterator(*cell, &dh);
      dh_cell->get_dof_indices(fe_dof_indices);
      const auto pic = inclusions_as_particles.particles_in_cell(cell);
      Assert(pic.begin() == particle, ExcInternalError());
      for (const auto &p : pic)
        {
          local_matrix          = 0;
          local_rhs             = 0;
          inclusion_dof_indices = inclusion->get_dof_indices(p.get_id());
          const auto &inclusion_fe_values =
            inclusion->reinit(p.get_id(), par.inclusions);

          const auto &ref_q  = p.get_reference_location();
          const auto &real_q = p.get_location();
          for (unsigned int j = 0; j < inclusion->n_coefficients; ++j)
            {
              for (unsigned int i = 0; i < fe->n_dofs_per_cell(); ++i)
                local_matrix(i, j) +=
                  fe->shape_value(i, ref_q) * inclusion_fe_values[j];
              local_rhs(j) +=
                inclusion_fe_values[j] * par.inclusions_rhs.value(real_q);
            }
          constraints.distribute_local_to_global(local_matrix,
                                                 fe_dof_indices,
                                                 inclusion_constraints,
                                                 inclusion_dof_indices,
                                                 coupling_matrix);
          inclusion_constraints.distribute_local_to_global(
            local_rhs, inclusion_dof_indices, system_rhs.block(1));
        }
      particle = pic.end();
    }
  coupling_matrix.compress(VectorOperation::add);
  system_rhs.compress(VectorOperation::add);
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::solve()
{
  TimerOutput::Scope       t(computing_timer, "Solve");
  LA::MPI::PreconditionAMG prec_A;
  {
    LA::MPI::PreconditionAMG::AdditionalData data;
#ifdef USE_PETSC_LA
    data.symmetric_operator = true;
#endif
    prec_A.initialize(stiffness_matrix, data);
  }

  const auto A    = linear_operator<LA::MPI::Vector>(stiffness_matrix);
  const auto amgA = linear_operator(A, prec_A);
  const auto Bt   = linear_operator<LA::MPI::Vector>(coupling_matrix);
  const auto B    = transpose_operator(Bt);

  ReductionControl solver_control_stiffness(1000, 1e-12, 1.e-8);

  LA::SolverCG cg_stiffness(solver_control_stiffness);
  const auto   invA = inverse_operator(A, cg_stiffness, amgA);

  // Schur complement
  const auto S = B * invA * Bt;

  ReductionControl solver_control(dh.n_dofs(), 1e-12, 1.e-8);
  LA::SolverCG     cg_schur(solver_control);
  LA::SolverGMRES  gmres_schur(solver_control);
  const auto       invS = inverse_operator(S, gmres_schur);

  auto &u      = solution.block(0);
  auto &lambda = solution.block(1);

  const auto &f = system_rhs.block(0);
  const auto &g = system_rhs.block(1);

  pcout << "   f norm: " << f.l2_norm() << ", g norm: " << g.l2_norm()
        << std::endl;

  lambda = invS * g;
  pcout << "   Solved for lambda in " << solver_control.last_step()
        << " iterations." << std::endl;

  u = invA * (f - Bt * lambda);

  pcout << "   Solved for u in " << solver_control.last_step() << " iterations."
        << std::endl;
  constraints.distribute(u);
  inclusion_constraints.distribute(lambda);
  locally_relevant_solution = solution;
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::refine_and_transfer()
{
  TimerOutput::Scope               t(computing_timer, "Refine");
  const FEValuesExtractors::Vector velocity(0);
  Vector<float>                    error_per_cell(tria.n_active_cells());
  KellyErrorEstimator<spacedim>::estimate(dh,
                                          QGauss<spacedim - 1>(par.fe_degree +
                                                               1),
                                          {},
                                          locally_relevant_solution,
                                          error_per_cell,
                                          fe->component_mask(velocity));
  if (par.refinement_strategy == "fixed_fraction")
    {
      parallel::distributed::GridRefinement::refine_and_coarsen_fixed_fraction(
        tria, error_per_cell, par.refinement_fraction, par.coarsening_fraction);
    }
  else if (par.refinement_strategy == "fixed_number")
    {
      parallel::distributed::GridRefinement::refine_and_coarsen_fixed_number(
        tria,
        error_per_cell,
        par.refinement_fraction,
        par.coarsening_fraction,
        par.max_cells);
    }
  for (const auto &cell : tria.active_cell_iterators())
    {
      if (cell->refine_flag_set() && cell->level() == par.max_level_refinement)
        cell->clear_refine_flag();
      if (cell->coarsen_flag_set() && cell->level() == par.min_level_refinement)
        cell->clear_coarsen_flag();
    }
  parallel::distributed::SolutionTransfer<spacedim, LA::MPI::BlockVector>
    transfer(dh);
  tria.prepare_coarsening_and_refinement();
  transfer.prepare_for_coarsening_and_refinement(locally_relevant_solution);
  tria.execute_coarsening_and_refinement();
  setup_dofs();
  transfer.interpolate(solution);
  constraints.distribute(solution);
  locally_relevant_solution = solution;
}
template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::output_solution() const
{
  TimerOutput::Scope t(computing_timer, "Output results");
  std::string        solution_name = "solution";
  DataOut<spacedim>  data_out;
  data_out.attach_dof_handler(dh);
  data_out.add_data_vector(locally_relevant_solution.block(0), solution_name);
  Vector<float> subdomain(tria.n_active_cells());
  for (unsigned int i = 0; i < subdomain.size(); ++i)
    subdomain(i) = tria.locally_owned_subdomain();
  data_out.add_data_vector(subdomain, "subdomain");
  data_out.build_patches();
  const std::string filename = "solution.vtu";
  data_out.write_vtu_in_parallel(par.output_directory + "/" + filename,
                                 mpi_communicator);
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::output_particles() const
{
  Particles::DataOut<spacedim> particles_out;
  particles_out.build_patches(inclusions_as_particles);
  const std::string filename = "particles.vtu";
  particles_out.write_vtu_in_parallel(par.output_directory + "/" + filename,
                                      mpi_communicator);
}



template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::print_parameters() const
{
#ifdef USE_PETSC_LA
  pcout << "Running PoissonProblem<" << Utilities::dim_string(dim, spacedim)
        << "> using PETSc." << std::endl;
#else
  pcout << "Running PoissonProblem<" << Utilities::dim_string(dim, spacedim)
        << "> using Trilinos." << std::endl;
#endif
  par.prm.print_parameters(par.output_directory + "/" + "used_parameters_" +
                             std::to_string(dim) + std::to_string(spacedim) +
                             ".prm",
                           ParameterHandler::Short);
}

template <int dim, int spacedim>
void
PoissonProblem<dim, spacedim>::run()
{
  print_parameters();
  setup_fe();
  make_grid();
  setup_inclusions_particles();
  output_particles();
  setup_dofs();
  assemble_poisson_system();
  assemble_coupling();
  solve();
  output_solution();
}

#endif