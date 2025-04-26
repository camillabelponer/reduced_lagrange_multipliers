// /* ---------------------------------------------------------------------
//  *
//  * Copyright (C) 2000 - 2020 by the deal.II authors
//  *
//  * This file is part of the deal.II library.
//  *
//  * The deal.II library is free software; you can use it, redistribute
//  * it, and/or modify it under the terms of the GNU Lesser General
//  * Public License as published by the Free Software Foundation; either
//  * version 2.1 of the License, or (at your option) any later version.
//  * The full text of the license can be found in the file LICENSE.md at
//  * the top level directory of deal.II.
//  *
//  * ---------------------------------------------------------------------
//  *
//  * Author: Wolfgang Bangerth, University of Heidelberg, 2000
//  * Modified by: Luca Heltai, 2020
//  */

// #include "laplacian.h"
// int
// main(int argc, char *argv[])
// {
//   using namespace dealii;
//   deallog.depth_console(10);
//   try
//     {
//       Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);
//       std::string                      prm_file;
//       if (argc > 1)
//         prm_file = argv[1];
//       else
//         prm_file = "parameters.prm";
//       if (prm_file.find("23d") != std::string::npos)
//         {
//           ProblemParameters<2, 3> par;
//           PoissonProblem<2, 3>    problem(par);
//           ParameterAcceptor::initialize(prm_file);
//           problem.run();
//         }
//       else if (prm_file.find("3d") != std::string::npos)
//         {
//           ProblemParameters<3> par;
//           PoissonProblem<3>    problem(par);
//           ParameterAcceptor::initialize(prm_file);
//           problem.run();
//         }
//       else
//         {
//           ProblemParameters<2> par;
//           PoissonProblem<2>    problem(par);
//           ParameterAcceptor::initialize(prm_file);
//           problem.run();
//         }
//     }
//   catch (std::exception &exc)
//     {
//       std::cerr << std::endl
//                 << std::endl
//                 << "----------------------------------------------------"
//                 << std::endl;
//       std::cerr << "Exception on processing: " << std::endl
//                 << exc.what() << std::endl
//                 << "Aborting!" << std::endl
//                 << "----------------------------------------------------"
//                 << std::endl;
//       return 1;
//     }
//   catch (...)
//     {
//       std::cerr << std::endl
//                 << std::endl
//                 << "----------------------------------------------------"
//                 << std::endl;
//       std::cerr << "Unknown exception!" << std::endl
//                 << "Aborting!" << std::endl
//                 << "----------------------------------------------------"
//                 << std::endl;
//       return 1;
//     }
//   return 0;
// }

// // ---------------------------------------------------------------------
// //
// // Copyright (C) 2024 by Luca Heltai
// //
// // This file is part of the reduced_lagrange_multipliers application, based
// on
// // the deal.II library.
// //
// // The reduced_lagrange_multipliers application is free software; you can use
// // it, redistribute it, and/or modify it under the terms of the Apache-2.0
// // License WITH LLVM-exception as published by the Free Software Foundation;
// // either version 3.0 of the License, or (at your option) any later version.
// The
// // full text of the license can be found in the file LICENSE.md at the top
// level
// // of the reduced_lagrange_multipliers distribution.
// //
// // ---------------------------------------------------------------------

// /* ---------------------------------------------------------------------
//  *
//  * Copyright (C) 2000 - 2020 by the deal.II authors
//  *
//  * This file is part of the deal.II library.
//  *
//  * The deal.II library is free software; you can use it, redistribute
//  * it, and/or modify it under the terms of the GNU Lesser General
//  * Public License as published by the Free Software Foundation; either
//  * version 2.1 of the License, or (at your option) any later version.
//  * The full text of the license can be found in the file LICENSE.md at
//  * the top level directory of deal.II.
//  *
//  * ---------------------------------------------------------------------
//  *
//  * Author: Wolfgang Bangerth, University of Heidelberg, 2000
//  * Modified by: Luca Heltai, 2020
//  */



#include "laplacian.h"

// #include <boost/algorithm/string.hpp>

// #include <cstdbool>

// #include "augmented_lagrangian.h"

// template <int dim, int spacedim>
// PoissonProblem<dim, spacedim>::PoissonProblem(
//   const ProblemParameters<dim, spacedim> &par)
//   : par(par)
//   , mpi_communicator(MPI_COMM_WORLD)
//   , pcout(std::cout, (Utilities::MPI::this_mpi_process(mpi_communicator) ==
//   0)) , computing_timer(mpi_communicator,
//                     pcout,
//                     TimerOutput::summary,
//                     TimerOutput::wall_times)
//   , tria(mpi_communicator,
//          typename Triangulation<spacedim>::MeshSmoothing(
//            Triangulation<spacedim>::smoothing_on_refinement |
//            Triangulation<spacedim>::smoothing_on_coarsening),
//          parallel::distributed::Triangulation<
//            spacedim>::construct_multigrid_hierarchy)
//   , dh(tria)
//   , mapping(1)
// {}


// template <int dim, int spacedim>
// void
// read_grid_and_cad_files(const std::string            &grid_file_name,
//                         const std::string            &ids_and_cad_file_names,
//                         Triangulation<dim, spacedim> &tria)
// {
//   GridIn<dim, spacedim> grid_in;
//   grid_in.attach_triangulation(tria);
//   grid_in.read(grid_file_name);
// #ifdef DEAL_II_WITH_OPENCASCADE
//   using map_type  = std::map<types::manifold_id, std::string>;
//   using Converter = Patterns::Tools::Convert<map_type>;
//   for (const auto &pair : Converter::to_value(ids_and_cad_file_names))
//     {
//       const auto &manifold_id   = pair.first;
//       const auto &cad_file_name = pair.second;
//       const auto  extension     = boost::to_lower_copy(
//         cad_file_name.substr(cad_file_name.find_last_of('.') + 1));
//       TopoDS_Shape shape;
//       if (extension == "iges" || extension == "igs")
//         shape = OpenCASCADE::read_IGES(cad_file_name);
//       else if (extension == "step" || extension == "stp")
//         shape = OpenCASCADE::read_STEP(cad_file_name);
//       else
//         AssertThrow(false,
//                     ExcNotImplemented("We found an extension that we "
//                                       "do not recognize as a CAD file "
//                                       "extension. Bailing out."));
//       const auto n_elements = OpenCASCADE::count_elements(shape);
//       if ((std::get<0>(n_elements) == 0))
//         tria.set_manifold(
//           manifold_id,
//           OpenCASCADE::ArclengthProjectionLineManifold<dim,
//           spacedim>(shape));
//       else if (spacedim == 3)
//         {
//           const auto t = reinterpret_cast<Triangulation<dim, 3> *>(&tria);
//           t->set_manifold(manifold_id,
//                           OpenCASCADE::NormalToMeshProjectionManifold<dim,
//                           3>(
//                             shape));
//         }
//       else
//         tria.set_manifold(manifold_id,
//                           OpenCASCADE::NURBSPatchManifold<dim, spacedim>(
//                             TopoDS::Face(shape)));
//     }
// #else
//   (void)ids_and_cad_file_names;
//   AssertThrow(false, ExcNotImplemented("Generation of the grid failed."));
// #endif
// }



// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::make_grid()
// {
//   try
//     {
//       GridGenerator::generate_from_name_and_arguments(tria,
//                                                       par.name_of_grid,
//                                                       par.arguments_for_grid);
//     }
//   catch (...)
//     {
//       pcout << "Generating from name and argument failed." << std::endl
//             << "Trying to read from file name." << std::endl;
//       read_grid_and_cad_files(par.name_of_grid, par.arguments_for_grid,
//       tria);
//     }
//   tria.refine_global(par.initial_refinement);
// }



// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::setup_fe()
// {
//   TimerOutput::Scope t(computing_timer, "Initial setup");
//   fe = std::make_unique<FESystem<spacedim>>(FE_Q<spacedim>(par.fe_degree),
//   1); quadrature = std::make_unique<QGauss<spacedim>>(par.fe_degree + 1);
// }


// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::setup_dofs()
// {
//   TimerOutput::Scope t(computing_timer, "Setup dofs");
//   dh.distribute_dofs(*fe);
// #ifdef MATRIX_FREE_PATH
//   dh.distribute_mg_dofs();
// #endif

//   owned_dofs.resize(2);
//   owned_dofs[0] = dh.locally_owned_dofs();
//   relevant_dofs.resize(2);
//   DoFTools::extract_locally_relevant_dofs(dh, relevant_dofs[0]);
//   {
//     constraints.reinit(owned_dofs[0], relevant_dofs[0]);
//     DoFTools::make_hanging_node_constraints(dh, constraints);
//     for (const auto id : par.dirichlet_ids)
//       VectorTools::interpolate_boundary_values(dh, id, par.bc, constraints);
//     constraints.close();
//   }
//   {
// #ifdef MATRIX_FREE_PATH
//     typename MatrixFree<spacedim, double>::AdditionalData additional_data;
//     additional_data.tasks_parallel_scheme =
//       MatrixFree<spacedim, double>::AdditionalData::none;
//     additional_data.mapping_update_flags =
//       (update_gradients | update_JxW_values | update_quadrature_points);
//     std::shared_ptr<MatrixFree<spacedim, double>> system_mf_storage(
//       new MatrixFree<spacedim, double>());
//     system_mf_storage->reinit(
//       mapping, dh, constraints, QGauss<1>(fe->degree + 1), additional_data);
//     stiffness_matrix.initialize(system_mf_storage);

//     // Perform setup for matrix-free multigrid
//     {
//       const unsigned int nlevels = tria.n_global_levels();
//       mg_matrices.resize(0, nlevels - 1);

//       const std::set<types::boundary_id> dirichlet_boundary_ids = {0};
//       mg_constrained_dofs.initialize(dh);
//       mg_constrained_dofs.make_zero_boundary_constraints(
//         dh, dirichlet_boundary_ids);

//       for (unsigned int level = 0; level < nlevels; ++level)
//         {
//           AffineConstraints<double> level_constraints(
//             dh.locally_owned_mg_dofs(level),
//             DoFTools::extract_locally_relevant_level_dofs(dh, level));
//           for (const types::global_dof_index dof_index :
//                mg_constrained_dofs.get_boundary_indices(level))
//             level_constraints.constrain_dof_to_zero(dof_index);
//           level_constraints.close();

//           typename MatrixFree<spacedim, float>::AdditionalData
//             additional_data_level;
//           additional_data_level.tasks_parallel_scheme =
//             MatrixFree<spacedim, float>::AdditionalData::none;
//           additional_data_level.mapping_update_flags =
//             (update_gradients | update_JxW_values | update_quadrature_points);
//           additional_data_level.mg_level = level;
//           std::shared_ptr<MatrixFree<spacedim, float>> mg_mf_storage_level =
//             std::make_shared<MatrixFree<spacedim, float>>();
//           mg_mf_storage_level->reinit(mapping,
//                                       dh,
//                                       level_constraints,
//                                       QGauss<1>(fe->degree + 1),
//                                       additional_data_level);

//           mg_matrices[level].initialize(mg_mf_storage_level,
//                                         mg_constrained_dofs,
//                                         level);
//         }
//     }


// #else
//     stiffness_matrix.clear();
//     DynamicSparsityPattern dsp(relevant_dofs[0]);
//     DoFTools::make_sparsity_pattern(dh, dsp, constraints, false);
//     SparsityTools::distribute_sparsity_pattern(dsp,
//                                                owned_dofs[0],
//                                                mpi_communicator,
//                                                relevant_dofs[0]);
//     stiffness_matrix.reinit(owned_dofs[0],
//                             owned_dofs[0],
//                             dsp,
//                             mpi_communicator);

// #endif
//   }
//   inclusion_constraints.close();

//   if (inclusions.n_dofs() > 0)
//     {
//       std::cout << "inclusions.n_inclusions() = " <<
//       inclusions.n_inclusions()
//                 << std::endl;
//       auto inclusions_set =
//         Utilities::MPI::create_evenly_distributed_partitioning(
//           mpi_communicator, inclusions.n_inclusions());

//       owned_dofs[1] = inclusions_set.tensor_product(
//         complete_index_set(inclusions.get_n_coefficients()));

//       coupling_matrix.clear();
//       DynamicSparsityPattern dsp(dh.n_dofs(),
//                                  inclusions.n_dofs(),
//                                  relevant_dofs[0]);

//       relevant_dofs[1] = assemble_coupling_sparsity(dsp);
//       SparsityTools::distribute_sparsity_pattern(dsp,
//                                                  owned_dofs[0],
//                                                  mpi_communicator,
//                                                  relevant_dofs[0]);
//       coupling_matrix.reinit(owned_dofs[0],
//                              owned_dofs[1],
//                              dsp,
//                              mpi_communicator);

//       DynamicSparsityPattern idsp(inclusions.n_dofs(),
//                                   inclusions.n_dofs(),
//                                   relevant_dofs[1]);
//       for (const auto i : relevant_dofs[1])
//         idsp.add(i, i);
//       SparsityTools::distribute_sparsity_pattern(idsp,
//                                                  owned_dofs[1],
//                                                  mpi_communicator,
//                                                  relevant_dofs[1]);
//       inclusion_matrix.reinit(owned_dofs[1],
//                               owned_dofs[1],
//                               idsp,
//                               mpi_communicator);

//       mass_matrix.reinit(owned_dofs[1], owned_dofs[1], idsp,
//       mpi_communicator);
//     }

//   locally_relevant_solution.reinit(owned_dofs, relevant_dofs,
//   mpi_communicator);

// #ifdef MATRIX_FREE_PATH
//   system_rhs.reinit(owned_dofs, relevant_dofs, mpi_communicator);
//   solution.reinit(owned_dofs, relevant_dofs, mpi_communicator);
// #else
//   system_rhs.reinit(owned_dofs, mpi_communicator);
//   solution.reinit(owned_dofs, mpi_communicator);
// #endif

//   pcout << "   Number of degrees of freedom: " << owned_dofs[0].size() << " +
//   "
//         << owned_dofs[1].size()
//         << " (locally owned: " << owned_dofs[0].n_elements() << " + "
//         << owned_dofs[1].n_elements() << ")" << std::endl;
// }


// #ifndef MATRIX_FREE_PATH
// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::assemble_poisson_system()
// {
//   stiffness_matrix = 0;
//   coupling_matrix  = 0;
//   system_rhs       = 0;
//   FEValues<spacedim>               fe_values(*fe,
//                                *quadrature,
//                                update_values | update_gradients |
//                                  update_quadrature_points | update_JxW_values);
//   const unsigned int               dofs_per_cell = fe->n_dofs_per_cell();
//   const unsigned int               n_q_points    = quadrature->size();
//   FullMatrix<double>               cell_matrix(dofs_per_cell, dofs_per_cell);
//   Vector<double>                   cell_rhs(dofs_per_cell);
//   std::vector<double>              rhs_values(n_q_points);
//   std::vector<Tensor<1, spacedim>> grad_phi_u(dofs_per_cell);
//   std::vector<types::global_dof_index> local_dof_indices(dofs_per_cell);
//   const FEValuesExtractors::Scalar     scalar(0);
//   for (const auto &cell : dh.active_cell_iterators())
//     if (cell->is_locally_owned())
//       {
//         cell_matrix = 0;
//         cell_rhs    = 0;
//         fe_values.reinit(cell);
//         par.rhs.value_list(fe_values.get_quadrature_points(), rhs_values);
//         for (unsigned int q = 0; q < n_q_points; ++q)
//           {
//             for (unsigned int k = 0; k < dofs_per_cell; ++k)
//               grad_phi_u[k] = fe_values[scalar].gradient(k, q);
//             for (unsigned int i = 0; i < dofs_per_cell; ++i)
//               {
//                 for (unsigned int j = 0; j < dofs_per_cell; ++j)
//                   {
//                     cell_matrix(i, j) +=
//                       grad_phi_u[i] * grad_phi_u[j] * fe_values.JxW(q);
//                   }
//                 cell_rhs(i) += fe_values.shape_value(i, q) * rhs_values[q] *
//                                fe_values.JxW(q);
//               }
//           }
//         cell->get_dof_indices(local_dof_indices);
//         constraints.distribute_local_to_global(cell_matrix,
//                                                cell_rhs,
//                                                local_dof_indices,
//                                                stiffness_matrix,
//                                                system_rhs.block(0));
//       }
//   stiffness_matrix.compress(VectorOperation::add);
//   system_rhs.compress(VectorOperation::add);
// }
// #endif


// template <int dim, int spacedim>
// IndexSet
// PoissonProblem<dim, spacedim>::assemble_coupling_sparsity(
//   DynamicSparsityPattern &dsp) const
// {
//   TimerOutput::Scope t(computing_timer, "Assemble Coupling sparsity");
//   IndexSet           relevant(inclusions.n_dofs());

//   std::vector<types::global_dof_index> dof_indices(fe->n_dofs_per_cell());
//   std::vector<types::global_dof_index> inclusion_dof_indices;

//   auto particle = inclusions.inclusions_as_particles.begin();
//   while (particle != inclusions.inclusions_as_particles.end())
//     {
//       const auto &cell = particle->get_surrounding_cell();
//       const auto  dh_cell =
//         typename DoFHandler<spacedim>::cell_iterator(*cell, &dh);
//       dh_cell->get_dof_indices(dof_indices);

//       const auto pic =
//         inclusions.inclusions_as_particles.particles_in_cell(cell);
//       Assert(pic.begin() == particle, ExcInternalError());
//       std::set<types::global_dof_index> inclusion_dof_indices_set;
//       for (const auto &p : pic)
//         {
//           const auto ids = inclusions.get_dof_indices(p.get_id());
//           inclusion_dof_indices_set.insert(ids.begin(), ids.end());
//         }
//       inclusion_dof_indices.resize(0);
//       inclusion_dof_indices.insert(inclusion_dof_indices.begin(),
//                                    inclusion_dof_indices_set.begin(),
//                                    inclusion_dof_indices_set.end());

//       constraints.add_entries_local_to_global(dof_indices,
//                                               inclusion_constraints,
//                                               inclusion_dof_indices,
//                                               dsp);

//       relevant.add_indices(inclusion_dof_indices.begin(),
//                            inclusion_dof_indices.end());
//       particle = pic.end();
//     }
//   return relevant;
// }



// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::assemble_coupling()
// {
//   TimerOutput::Scope t(computing_timer, "Assemble Coupling matrix");
//   pcout << "Assemble coupling matrix. " << std::endl;

//   std::vector<types::global_dof_index> fe_dof_indices(fe->n_dofs_per_cell());
//   std::vector<types::global_dof_index> inclusion_dof_indices(
//     inclusions.get_n_coefficients());

//   FullMatrix<double> local_coupling_matrix(fe->n_dofs_per_cell(),
//                                            inclusions.get_n_coefficients());

//   [[maybe_unused]] FullMatrix<double>
//   local_bulk_matrix(fe->n_dofs_per_cell(),
//                                                         fe->n_dofs_per_cell());

//   FullMatrix<double> local_inclusion_matrix(inclusions.get_n_coefficients(),
//                                             inclusions.get_n_coefficients());
//   FullMatrix<double> local_mass_matrix(inclusions.get_n_coefficients(),
//                                        inclusions.get_n_coefficients());

//   Vector<double> local_rhs(inclusions.get_n_coefficients());

//   auto particle = inclusions.inclusions_as_particles.begin();
//   while (particle != inclusions.inclusions_as_particles.end())
//     {
//       const auto &cell = particle->get_surrounding_cell();
//       const auto  dh_cell =
//         typename DoFHandler<spacedim>::cell_iterator(*cell, &dh);
//       dh_cell->get_dof_indices(fe_dof_indices);
//       const auto pic =
//         inclusions.inclusions_as_particles.particles_in_cell(cell);
//       Assert(pic.begin() == particle, ExcInternalError());

//       auto p      = pic.begin();
//       auto next_p = pic.begin();
//       while (p != pic.end())
//         {
//           const auto inclusion_id = inclusions.get_inclusion_id(p->get_id());
//           inclusion_dof_indices   = inclusions.get_dof_indices(p->get_id());
//           local_coupling_matrix   = 0;
//           local_inclusion_matrix  = 0;
//           local_mass_matrix       = 0;
//           local_rhs               = 0;

//           // Extract all points that refer to the same inclusion
//           std::vector<Point<spacedim>> ref_q_points;
//           for (; next_p != pic.end() &&
//                  inclusions.get_inclusion_id(next_p->get_id()) ==
//                  inclusion_id;
//                ++next_p)
//             ref_q_points.push_back(next_p->get_reference_location());
//           FEValues<spacedim, spacedim> fev(*fe,
//                                            ref_q_points,
//                                            update_values | update_gradients);
//           fev.reinit(dh_cell);
//           for (unsigned int q = 0; q < ref_q_points.size(); ++q)
//             {
//               const auto  id                  = p->get_id();
//               const auto &inclusion_fe_values = inclusions.get_fe_values(id);
//               const auto &real_q              = p->get_location();

//               // Coupling and inclusions matrix
//               for (unsigned int j = 0; j < inclusions.n_dofs_per_inclusion();
//                    ++j)
//                 {
//                   for (unsigned int i = 0; i < fe->n_dofs_per_cell(); ++i)
//                     local_coupling_matrix(i, j) +=
//                       (fev.shape_value(i, q)) * inclusion_fe_values[j];

//                   local_rhs(j) +=
//                     inclusion_fe_values[j] *
//                     inclusions.get_inclusion_data(inclusion_id, j, real_q);

//                   local_inclusion_matrix(j, j) +=
//                     (inclusion_fe_values[j] * inclusion_fe_values[j] /
//                      inclusion_fe_values[0]);

//                   local_mass_matrix(j, j) += inclusion_fe_values[j] *
//                                              inclusion_fe_values[j] /
//                                              inclusion_fe_values[0];
//                 }
//               ++p;
//             }
//           // I expect p and next_p to be the same now.
//           Assert(p == next_p, ExcInternalError());

//           // Add local matrices to global ones
//           constraints.distribute_local_to_global(local_coupling_matrix,
//                                                  fe_dof_indices,
//                                                  inclusion_constraints,
//                                                  inclusion_dof_indices,
//                                                  coupling_matrix);
//           inclusion_constraints.distribute_local_to_global(
//             local_rhs, inclusion_dof_indices, system_rhs.block(1));

//           inclusion_constraints.distribute_local_to_global(
//             local_inclusion_matrix, inclusion_dof_indices, inclusion_matrix);

//           inclusion_constraints.distribute_local_to_global(
//             local_mass_matrix, inclusion_dof_indices, mass_matrix);
//         }
//       particle = pic.end();
//     }
//   coupling_matrix.compress(VectorOperation::add);
//   mass_matrix.compress(VectorOperation::add);
// #ifndef MATRIX_FREE_PATH
//   stiffness_matrix.compress(VectorOperation::add);
// #endif
//   inclusion_matrix.compress(VectorOperation::add);
//   system_rhs.compress(VectorOperation::add);
//   pcout << "System rhs: " << system_rhs.l2_norm() << std::endl;
// }


// #ifdef MATRIX_FREE_PATH
// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::assemble_rhs()
// {
//   stiffness_matrix.get_matrix_free()->initialize_dof_vector(solution.block(0));
//   stiffness_matrix.get_matrix_free()->initialize_dof_vector(
//     system_rhs.block(0));
//   system_rhs        = 0;
//   solution.block(0) = 0;
//   constraints.distribute(solution.block(0));
//   solution.block(0).update_ghost_values();

//   FEEvaluation<spacedim, -1> phi(*stiffness_matrix.get_matrix_free());
//   for (unsigned int cell = 0;
//        cell < stiffness_matrix.get_matrix_free()->n_cell_batches();
//        ++cell)
//     {
//       phi.reinit(cell);
//       phi.read_dof_values_plain(solution.block(0));
//       phi.evaluate(EvaluationFlags::gradients);
//       for (unsigned int q = 0; q < phi.n_q_points; ++q)
//         {
//           const Point<spacedim, VectorizedArray<double>> p_vect =
//             phi.quadrature_point(q);

//           VectorizedArray<double> f_value = 0.0;
//           for (unsigned int v = 0; v < VectorizedArray<double>::size(); ++v)
//             {
//               Point<spacedim> p;
//               for (unsigned int d = 0; d < spacedim; ++d)
//                 p[d] = p_vect[d][v];
//               f_value[v] = par.rhs.value(p);
//             }
//           phi.submit_gradient(-phi.get_gradient(q), q);
//           phi.submit_value(f_value, q);
//         }
//       phi.integrate(EvaluationFlags::values | EvaluationFlags::gradients);
//       phi.distribute_local_to_global(system_rhs.block(0));
//     }

//   system_rhs.compress(VectorOperation::add);
// }
// #endif


// void
// output_double_number(double input, const std::string &text)
// {
//   std::cout << text << input << std::endl;
// }

// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::solve()
// {
//   TimerOutput::Scope t(computing_timer, "Solve");
//   pcout << "Preparing solve." << std::endl;
//   SolverCG<VectorType> cg_stiffness(par.inner_control);
// #ifdef MATRIX_FREE_PATH

//   using Payload =
//   dealii::internal::LinearOperatorImplementation::EmptyPayload;
//   LinearOperator<VectorType, VectorType, Payload> A;
//   A = linear_operator<VectorType, VectorType, Payload>(stiffness_matrix);

//   MGTransferMatrixFree<spacedim, float> mg_transfer(mg_constrained_dofs);
//   mg_transfer.build(dh);

//   using SmootherType =
//     PreconditionChebyshev<LevelMatrixType,
//                           LinearAlgebra::distributed::Vector<float>>;
//   mg::SmootherRelaxation<SmootherType,
//                          LinearAlgebra::distributed::Vector<float>>
//                                                        mg_smoother;
//   MGLevelObject<typename SmootherType::AdditionalData> smoother_data;
//   smoother_data.resize(0, tria.n_global_levels() - 1);
//   for (unsigned int level = 0; level < tria.n_global_levels(); ++level)
//     {
//       if (level > 0)
//         {
//           smoother_data[level].smoothing_range     = 15.;
//           smoother_data[level].degree              = 5;
//           smoother_data[level].eig_cg_n_iterations = 10;
//         }
//       else
//         {
//           smoother_data[0].smoothing_range     = 1e-3;
//           smoother_data[0].degree              =
//           numbers::invalid_unsigned_int; smoother_data[0].eig_cg_n_iterations
//           = mg_matrices[0].m();
//         }
//       mg_matrices[level].compute_diagonal();
//       smoother_data[level].preconditioner =
//         mg_matrices[level].get_matrix_diagonal_inverse();
//     }
//   mg_smoother.initialize(mg_matrices, smoother_data);

//   MGCoarseGridApplySmoother<LinearAlgebra::distributed::Vector<float>>
//     mg_coarse;
//   mg_coarse.initialize(mg_smoother);

//   mg::Matrix<LinearAlgebra::distributed::Vector<float>>
//   mg_matrix(mg_matrices);

//   MGLevelObject<MatrixFreeOperators::MGInterfaceOperator<LevelMatrixType>>
//     mg_interface_matrices;
//   mg_interface_matrices.resize(0, tria.n_global_levels() - 1);
//   for (unsigned int level = 0; level < tria.n_global_levels(); ++level)
//     mg_interface_matrices[level].initialize(mg_matrices[level]);
//   mg::Matrix<LinearAlgebra::distributed::Vector<float>> mg_interface(
//     mg_interface_matrices);

//   Multigrid<LinearAlgebra::distributed::Vector<float>> mg(
//     mg_matrix, mg_coarse, mg_transfer, mg_smoother, mg_smoother);
//   mg.set_edge_matrices(mg_interface, mg_interface);

//   PreconditionMG<spacedim,
//                  LinearAlgebra::distributed::Vector<float>,
//                  MGTransferMatrixFree<spacedim, float>>
//     preconditioner(dh, mg, mg_transfer);

//   auto invA = A;
//   invA      = inverse_operator(A, cg_stiffness, preconditioner);
// #else
//   using Payload =
//     TrilinosWrappers::internal::LinearOperatorImplementation::TrilinosPayload;

//   auto A = linear_operator<VectorType, VectorType,
//   Payload>(stiffness_matrix);

//   PreconditionAMG prec_A;
//   {
//     PreconditionAMG::AdditionalData data;
// #  ifdef USE_PETSC_LA
//     data.symmetric_operator = true;
// #  endif
//     prec_A.initialize(stiffness_matrix, data);
//   }
//   const auto amgA = linear_operator<VectorType, VectorType>(A, prec_A);
//   auto       invA = inverse_operator(A, cg_stiffness, amgA);
// #endif


//   // Some aliases
//   auto &u = solution.block(0);
//   // auto &lambda = solution.block(1);

//   const auto &f = system_rhs.block(0);
//   // const auto &g = system_rhs.block(1);

//   if (inclusions.n_dofs() == 0)
//     {
//       u = invA * f;
//     }
//   else
//     {
// #ifdef MATRIX_FREE_PATH
//       auto Bt =
//         linear_operator<VectorType, VectorType, Payload>(*coupling_operator);
//       Bt.reinit_range_vector = [this](VectorType &vec, const bool) {
//         vec.reinit(owned_dofs[0], relevant_dofs[0], mpi_communicator);
//       };
//       Bt.reinit_domain_vector = [this](VectorType &vec, const bool) {
//         vec.reinit(owned_dofs[1], relevant_dofs[1], mpi_communicator);
//       };

//       const auto B = transpose_operator<VectorType, VectorType, Payload>(Bt);
// #else
//       const auto Bt =
//         linear_operator<VectorType, VectorType, Payload>(coupling_matrix);
//       const auto B = transpose_operator<VectorType, VectorType, Payload>(Bt);
// #endif
//       const auto C =
//         linear_operator<VectorType, VectorType, Payload>(inclusion_matrix);


//       {
//         // Estimate condition number:
//         std::cout << "- - - - - - - - - - - - - - - - - - - - - - - -"
//                   << std::endl;
//         std::cout << "Estimate condition number of CCt using CG" <<
//         std::endl; SolverControl             solver_control(2000, 1e-12);
//         SolverCG<Vector> solver_cg(solver_control);

//         solver_cg.connect_condition_number_slot(
//           std::bind(output_double_number,
//                     std::placeholders::_1,
//                     "Condition number estimate: "));

//         auto CCt = B * Bt;

//         Vector u;
//         u.reinit(system_rhs.block(1));
//         u = 0.;

//         Vector f;
//         f.reinit(system_rhs.block(1));
//         f = 1.;
//         PreconditionIdentity prec_no;
//         try
//           {
//             solver_cg.solve(CCt, u, f, prec_no);
//           }
//         catch (...)
//           {
//             std::cerr
//               << "***CCt solve not successfull (see condition number
//               above)***"
//               << std::endl;
//           }
//       }


// #ifdef FALSE
//       // Schur complement
//       pcout << "   Prepare schur... ";
//       const auto S = B * invA * Bt;
//       pcout << "S was built." << std::endl;

//       // Schur complement preconditioner
//       auto                 invS = S;
//       SolverCG<VectorType> cg_schur(par.outer_control);
//       invS = inverse_operator<Payload, SolverCG<VectorType>>(S, cg_schur);

//       pcout << "   f norm: " << f.l2_norm() << ", g norm: " << g.l2_norm()
//             << std::endl;

//       // Compute Lambda first
//       lambda = invS * (B * invA * f - g);
//       pcout << "   Solved for lambda in " << par.outer_control.last_step()
//             << " iterations." << std::endl;

//       // Then compute u
//       u = invA * (f - Bt * lambda);
//       pcout << "   u norm: " << u.l2_norm()
//             << ", lambda norm: " << lambda.l2_norm() << std::endl;

//       pcout << "   Solved for u in " << par.inner_control.last_step()
//             << " iterations." << std::endl;

// #endif

//       const auto M = linear_operator<Vector>(mass_matrix);

//       {
//         TrilinosWrappers::PreconditionILU M_inv_ilu;
//         M_inv_ilu.initialize(mass_matrix);

//         SolverControl solver_control(100, 1e-15, false, false);
//         SolverCG<TrilinosWrappers::MPI::Vector> solver_CG_M(solver_control);
//         auto invM = inverse_operator(M, solver_CG_M, M_inv_ilu);
//         auto invW = invM * invM;

//         // Try augmented lagrangian preconditioner
//         const double gamma = 10;
//         auto         Aug   = A + gamma * Bt * invW * B;


//         auto Zero = M * 0.0;
//         auto AA   = block_operator<2, 2, BlockVector>(
//           {{{{Aug, Bt}}, {{B, Zero}}}}); //! Augmented the (1,1) block

//         BlockVector solution_block;
//         BlockVector system_rhs_block;
//         AA.reinit_domain_vector(solution_block, false);
//         AA.reinit_range_vector(system_rhs_block, false);


//         // lagrangian term
//         Vector tmp;
//         tmp.reinit(system_rhs.block(0));
//         tmp                       = gamma * Bt * invW * system_rhs.block(1);
//         system_rhs_block.block(0) = system_rhs.block(0);
//         system_rhs_block.block(0).add(1., tmp); // ! augmented
//         system_rhs_block.block(1) = system_rhs.block(1);

//         SolverControl             control_lagrangian(100000, 1e-2, false,
//         true); SolverCG<Vector>
//         solver_lagrangian(control_lagrangian);

//         auto                               Aug_inv = inverse_operator(Aug,
//                                         solver_lagrangian); //! augmented
//         SolverFGMRES<BlockVector> solver_fgmres(par.outer_control);

//         BlockPreconditionerAugmentedLagrangian<Vector>
//           augmented_lagrangian_preconditioner{Aug_inv, B, Bt, invW, gamma};

//         solver_fgmres.solve(AA,
//                             solution_block,
//                             system_rhs_block,
//                             augmented_lagrangian_preconditioner);

//         pcout << "Solver with FGMRES in " << par.outer_control.last_step()
//               << " iterations." << std::endl;

//         solution.block(0) = solution_block.block(0);

//         constraints.distribute(solution_block.block(0));
//       }
//     }


//   constraints.distribute(solution.block(0));
//   inclusion_constraints.distribute(solution.block(1));
//   solution.update_ghost_values();
//   locally_relevant_solution = solution;
// }



// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::refine_and_transfer()
// {
//   TimerOutput::Scope t(computing_timer, "Refine");
//   Vector<float>      error_per_cell(tria.n_active_cells());
//   KellyErrorEstimator<spacedim>::estimate(dh,
//                                           QGauss<spacedim - 1>(par.fe_degree
//                                           +
//                                                                1),
//                                           {},
//                                           locally_relevant_solution.block(0),
//                                           error_per_cell);
//   if (par.refinement_strategy == "fixed_fraction")
//     {
//       parallel::distributed::GridRefinement::refine_and_coarsen_fixed_fraction(
//         tria, error_per_cell, par.refinement_fraction,
//         par.coarsening_fraction);
//     }
//   else if (par.refinement_strategy == "fixed_number")
//     {
//       parallel::distributed::GridRefinement::refine_and_coarsen_fixed_number(
//         tria,
//         error_per_cell,
//         par.refinement_fraction,
//         par.coarsening_fraction,
//         par.max_cells);
//     }
//   else if (par.refinement_strategy == "global")
//     for (const auto &cell : tria.active_cell_iterators())
//       cell->set_refine_flag();

//   parallel::distributed::SolutionTransfer<spacedim, VectorType> transfer(dh);
//   tria.prepare_coarsening_and_refinement();
//   inclusions.inclusions_as_particles.prepare_for_coarsening_and_refinement();
//   transfer.prepare_for_coarsening_and_refinement(
//     locally_relevant_solution.block(0));
//   tria.execute_coarsening_and_refinement();
//   inclusions.inclusions_as_particles.unpack_after_coarsening_and_refinement();
//   setup_dofs();
//   transfer.interpolate(solution.block(0));
//   constraints.distribute(solution.block(0));
//   locally_relevant_solution.block(0) = solution.block(0);
// }



// template <int dim, int spacedim>
// std::string
// PoissonProblem<dim, spacedim>::output_solution() const
// {
//   TimerOutput::Scope t(computing_timer, "Output results");
//   std::string        solution_name = "solution";
//   DataOut<spacedim>  data_out;
//   data_out.attach_dof_handler(dh);
//   data_out.add_data_vector(locally_relevant_solution.block(0),
//   solution_name); Vector<float> subdomain(tria.n_active_cells()); for
//   (unsigned int i = 0; i < subdomain.size(); ++i)
//     subdomain(i) = tria.locally_owned_subdomain();
//   data_out.add_data_vector(subdomain, "subdomain");
//   data_out.build_patches();
//   const std::string filename =
//     par.output_name + "_" + std::to_string(cycle) + ".vtu";
//   data_out.write_vtu_in_parallel(par.output_directory + "/" + filename,
//                                  mpi_communicator);
//   return filename;
// }



// // template <int dim, int spacedim>
// // std::string
// // PoissonProblem<dim, spacedim>::output_particles() const
// // {
// //   Particles::DataOut<spacedim> particles_out;
// //   particles_out.build_patches(inclusions.inclusions_as_particles);
// //   const std::string filename =
// //     par.output_name + "_particles_" + std::to_string(cycle) + ".vtu";
// //   particles_out.write_vtu_in_parallel(par.output_directory + "/" +
// //   filename,
// //                                       mpi_communicator);
// //   return filename;
// // }


// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::output_results() const
// {
//   static std::vector<std::pair<double, std::string>> cycles_and_solutions;
//   static std::vector<std::pair<double, std::string>> cycles_and_particles;

//   if (cycles_and_solutions.size() == cycle)
//     {
//       cycles_and_solutions.push_back({(double)cycle, output_solution()});

//       const std::string particles_filename =
//         par.output_name + "_particles_" + std::to_string(cycle) + ".vtu";
//       inclusions.output_particles(par.output_directory + "/" +
//                                   particles_filename);

//       cycles_and_particles.push_back({(double)cycle, particles_filename});

//       std::ofstream pvd_solutions(par.output_directory + "/" +
//       par.output_name +
//                                   ".pvd");
//       std::ofstream pvd_particles(par.output_directory + "/" +
//       par.output_name +
//                                   "_particles.pvd");
//       DataOutBase::write_pvd_record(pvd_solutions, cycles_and_solutions);
//       DataOutBase::write_pvd_record(pvd_particles, cycles_and_particles);
//     }
// }

// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::print_parameters() const
// {
// #ifdef USE_PETSC_LA
//   pcout << "Running PoissonProblem<" << Utilities::dim_string(dim, spacedim)
//         << "> using PETSc." << std::endl;
// #else
//   pcout << "Running PoissonProblem<" << Utilities::dim_string(dim, spacedim)
//         << "> using Trilinos." << std::endl;
// #endif
//   par.prm.print_parameters(par.output_directory + "/" + "used_parameters_" +
//                              std::to_string(dim) + std::to_string(spacedim) +
//                              ".prm",
//                            ParameterHandler::Short);
// }

// template <int dim, int spacedim>
// void
// PoissonProblem<dim, spacedim>::run()
// {
//   std::cout << "Try to read the vtk" << std::endl;
//   GridIn<1, 3>        grid_in;
//   Triangulation<1, 3> tria_test;
//   grid_in.attach_triangulation(tria_test);
//   std::ifstream input_file("../notebooks/tree.vtk");
//   grid_in.read_vtk(input_file);
//   DoFHandler<1, 3> dh_test(tria_test);
//   FE_Q<1, 3>       fe{1};
//   dh_test.distribute_dofs(fe);
//   std::cout << "dh_test.n_dofs() = " << dh_test.n_dofs() << std::endl;

//   std::cout << "tria.n_active_cells() = " << tria_test.n_active_cells()
//             << std::endl;
//   return;

//   print_parameters();
//   make_grid();
//   setup_fe();
//   for (cycle = 0; cycle < par.n_refinement_cycles; ++cycle)
//     {
//       inclusions.setup_inclusions_particles(tria);
//       std::cout << "Number of global particles: "
//                 << inclusions.inclusions_as_particles.n_global_particles()
//                 << std::endl;
//       setup_dofs();
//       if (par.output_results_before_solving)
//         output_results();
// #ifdef MATRIX_FREE_PATH
//       assemble_rhs();
// #else
//       assemble_poisson_system();
// #endif
//       assemble_coupling();
// #ifdef MATRIX_FREE_PATH
//       MappingQ1<spacedim> mapping;
//       coupling_operator = std::make_unique<CouplingOperator<spacedim,
//       double>>(
//         inclusions, dh, constraints, mapping, *fe);
// #endif
//       // return;
//       solve();
//       output_results();
//       par.convergence_table.error_from_exact(dh,
//                                              locally_relevant_solution.block(0),
//                                              par.bc);
//       if (cycle != par.n_refinement_cycles - 1)
//         refine_and_transfer();
//       if (pcout.is_active())
//         par.convergence_table.output_table(pcout.get_stream());
//     }
// }


// // Template instantiations
// template class PoissonProblem<2>;
// template class PoissonProblem<2, 3>;
// template class PoissonProblem<3>;

// ------------------------------------------------------------------------
//
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2018 - 2020 by the deal.II authors
//
// This file is part of the deal.II library.
//
// Part of the source code is dual licensed under Apache-2.0 WITH
// LLVM-exception OR LGPL-2.1-or-later. Detailed license information
// governing the source code and code contributions can be found in
// LICENSE.md and CONTRIBUTING.md at the top level directory of deal.II.
//
// ------------------------------------------------------------------------

#include <deal.II/base/function_lib.h>
#include <deal.II/base/point.h>
#include <deal.II/base/tensor.h>

#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_out.h>

#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparsity_pattern.h>

#include <deal.II/non_matching/coupling.h>

#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/vector_tools.h>



// Test that a coupling matrix can be constructed for each pair of dimension
// and immersed dimension, and check that quadratic functions are correctly
// projected.

void
output_double_number(double input, const std::string &text)
{
  std::cout << text << input << std::endl;
}


template <int dim, int spacedim>
void
test()
{
  std::cout << "dim: " << dim << ", spacedim: " << spacedim << std::endl;

  Triangulation<dim, spacedim>      tria;
  Triangulation<spacedim, spacedim> space_tria;

  std::cout << "Try to read the vtk" << std::endl;
  GridIn<dim, spacedim> grid_in;
  grid_in.attach_triangulation(tria);
  // std::ifstream input_file("../notebooks/tree.vtk");
  std::string   filename = SOURCE_DIR "/data/tests/mstree_1000.vtk";
  std::ifstream input_file(filename);
  grid_in.read_vtk(input_file);

  GridGenerator::hyper_cube(space_tria, -.4, 1.2);

  // tria.refine_global(1);
  space_tria.refine_global(5);
  GridOut       go;
  std::ofstream output_name{"space_tria.vtk"};
  go.write_vtk(space_tria, output_name);

  FE_Q<dim, spacedim>      fe(1);
  FE_Q<spacedim, spacedim> space_fe(1);

  std::cout << "FE      : " << fe.get_name() << std::endl
            << "Space FE: " << space_fe.get_name() << std::endl;

  DoFHandler<dim, spacedim>      dh(tria);
  DoFHandler<spacedim, spacedim> space_dh(space_tria);

  dh.distribute_dofs(fe);
  space_dh.distribute_dofs(space_fe);

  std::cout << "Dofs      : " << dh.n_dofs() << std::endl
            << "Space dofs: " << space_dh.n_dofs() << std::endl;

  QGauss<dim> quad(3); // Quadrature for coupling


  SparsityPattern sparsity;
  {
    DynamicSparsityPattern dsp(space_dh.n_dofs(), dh.n_dofs());
    NonMatching::create_coupling_sparsity_pattern(space_dh, dh, quad, dsp);
    sparsity.copy_from(dsp);
  }
  SparseMatrix<double> coupling(sparsity);
  NonMatching::create_coupling_mass_matrix(
    space_dh, dh, quad, coupling, AffineConstraints<double>());

  SparsityPattern mass_sparsity;
  {
    DynamicSparsityPattern dsp(dh.n_dofs(), dh.n_dofs());
    DoFTools::make_sparsity_pattern(dh, dsp);
    mass_sparsity.copy_from(dsp);
  }
  SparseMatrix<double> mass_matrix(mass_sparsity);
  MatrixTools::create_mass_matrix(dh, quad, mass_matrix);

  SparseDirectUMFPACK mass_matrix_inv;
  mass_matrix_inv.factorize(mass_matrix);

  // now take the square function in space, project them onto the immersed
  // space, get back ones, and check for the error.
  Vector<double> space_square(space_dh.n_dofs());
  Vector<double> squares(dh.n_dofs());
  Vector<double> projected_squares(dh.n_dofs());

  VectorTools::interpolate(space_dh,
                           Functions::SquareFunction<spacedim>(),
                           space_square);
  VectorTools::interpolate(dh, Functions::SquareFunction<spacedim>(), squares);

  coupling.Tvmult(projected_squares, space_square);
  mass_matrix_inv.solve(projected_squares);

  projected_squares -= squares;

  std::cout << "Error on squares: " << projected_squares.l2_norm() << std::endl;


  {
    // Estimate condition number:
    std::cout << "- - - - - - - - - - - - - - - - - - - - - - - -" << std::endl;
    std::cout << "Estimate condition number of CCt using CG" << std::endl;
    SolverControl            solver_control(2000, 1e-12);
    SolverCG<Vector<double>> solver_cg(solver_control);

    solver_cg.connect_condition_number_slot(
      std::bind(output_double_number,
                std::placeholders::_1,
                "Condition number estimate: "));

    auto Bt  = linear_operator(coupling);
    auto B   = transpose_operator(Bt);
    auto CCt = B * Bt;

    Vector<double> u;
    u.reinit(coupling.n());
    u = 0.;

    Vector<double> f;
    f.reinit(coupling.n());
    f = 1.;
    PreconditionIdentity prec_no;
    try
      {
        solver_cg.solve(CCt, u, f, prec_no);
      }
    catch (...)
      {
        std::cerr
          << "***CCt solve not successfull (see condition number above)***"
          << std::endl;
      }
  }
}



int
main()
{
  // test<1, 1>();
  // test<1, 2>();
  // test<2, 2>();
  // test<2, 3>();
  test<1, 3>();
}