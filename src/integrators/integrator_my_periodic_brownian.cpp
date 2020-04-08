/* ***************************************************************************
 *
 *  Copyright (C) 2013-2016 University of Dundee
 *  All rights reserved. 
 *
 *  This file is part of SAMoS (Soft Active Matter on Surfaces) program.
 *
 *  SAMoS is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  SAMoS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ****************************************************************************/

/*!
 * \file integrator_brownian.cpp
 * \author Rastko Sknepnek, sknepnek@gmail.com
 * \date 07-Apr-2020
 * \brief Implementation of My Periodic Brownian dynamics integrator.
 */ 

#include "integrator_my_periodic_brownian.hpp"

/*! This function integrates equations of motion as introduced in the 
 *  Eqs. (1a) and (1b) of Y. Fily, et al., arXiv:1309.3714
 *  \f$ \partial_t \vec r_i = v_0 \hat{\vec n_i} + \mu \sum_{i\neq j}\vec F_{ij} \f$ and
 *  \f$ \partial_t \vartheta_i = \eta_i(t) \f$, where \f$ \mu \f$ is the mobility, 
 *  \f$ \vartheta_i \f$ defines orientation of the velocity in the tangent plane,
 *  \f$ \hat{\vec n}_i = \left(\cos\vartheta_i,\sin\vartheta_i\right) \f$, and
 *  \f$ \eta_i(t) \f$ is the Gaussian white noise with zero mean and delta function 
 *  correlations, \f$ \left<\eta_i(t)\eta_j(t')\right> = 2\nu_r\delta_{ij}\delta\left(t-t'\right) \f$, with
 *  \f$ \nu_r \f$ being the rotational diffusion rate.
**/
void IntegratorMyPeriodicBrownian::integrate()
{
  int N = m_system->get_group(m_group_name)->get_size();
  double T = m_temp->get_val(m_system->get_run_step());
  double B = sqrt(2.0*m_mu*T);
  double sqrt_dt = sqrt(m_dt);
  double fd_x, fd_y, fd_z;                    // Deterministic part of the force
  double fr_x = 0.0, fr_y = 0.0, fr_z = 0.0;  // Random part of the force
  vector<int> particles = m_system->get_group(m_group_name)->get_particles();
  
  // reset forces and torques
  m_system->reset_forces();
  m_system->reset_torques();
  
  // If nematic, attempt to flip directors
  if (m_nematic)
    for (int i = 0; i < N; i++)
    {
      int pi = particles[i];
      Particle& p = m_system->get_particle(pi);
      if (m_rng->drnd() < m_tau)  // Flip direction n with probability m_tua (dt/tau, where tau is the parameter given in the input file).
      {
        p.nx = -p.nx;  p.ny = -p.ny;  p.nz = -p.nz;
        if (m_velocity)
        {
          p.vx = -p.vx;  p.vy = -p.vy;  p.vz = -p.vz;
        }
      }
    }
  
  // compute forces in the current configuration
  if (m_potential)
    m_potential->compute(m_dt);
  // compute torques in the current configuration
  if (m_align)
    m_align->compute();
  // iterate over all particles 
  for (int i = 0; i < N; i++)
  {
    int pi = particles[i];
    Particle& p = m_system->get_particle(pi);
    // compute deterministic forces
    fd_x = m_v0*p.nx + m_mu*p.fx; 
    fd_y = m_v0*p.ny + m_mu*p.fy;
    fd_z = m_v0*p.nz + m_mu*p.fz;
    // Update velocity
    p.vx = fd_x; 
    p.vy = fd_y;
    p.vz = fd_z;
    // Update particle position according to the eq. (1a)
    p.x += m_dt*fd_x;
    p.y += m_dt*fd_y;
    p.z += m_dt*fd_z;
    // Check is non-zero T
    if (T > 0.0)
    {
      fr_x = B*m_rng->gauss_rng(1.0);
      fr_y = B*m_rng->gauss_rng(1.0);
      fr_z = B*m_rng->gauss_rng(1.0);
      p.vx += fr_x; 
      p.vy += fr_y;
      p.vz += fr_z;  
      p.x += sqrt_dt*fr_x;
      p.y += sqrt_dt*fr_y;
      p.z += sqrt_dt*fr_z;
    }
    // Project everything back to the manifold
    m_constrainer->enforce(p);
    // Update angular velocity
    p.omega = m_mur*m_constrainer->project_torque(p);
    //p.omega = m_dt*m_constraint->project_torque(p);
    // Change orientation of the director (in the tangent plane) according to eq. (1b)
    double dtheta = m_dt*p.omega + m_stoch_coeff*m_rng->gauss_rng(1.0);
    //double dtheta = m_dt*m_constraint->project_torque(p) + m_stoch_coeff*m_rng->gauss_rng(1.0);
    m_constrainer->rotate_director(p,dtheta);
    if (m_velocity)
      m_constrainer->rotate_velocity(p,dtheta);  
    //p.omega = dtheta*m_dt;
    p.age += m_dt;
  }
  // Begin of my changes
  bool periodic_update = true;
  bool update_boundary = false;
  if (periodic_update)
  {
      double length_period = 30.0;
      double bpacking = 2.0;
      double x_lenghth = 100.0;
      int internal_index_start = 0;
      int internal_index_end = 2904/3-1;
      int up_internal_index_start = internal_index_end+1;
      int up_internal_index_end = up_internal_index_start+(internal_index_end-internal_index_start);
      int down_internal_index_start = up_internal_index_end+1;
      int down_internal_index_end = down_internal_index_start+(internal_index_end-internal_index_start);

      int right_boundary_start = int(internal_index_end+length_period*bpacking);
      int right_boundary_end = int(internal_index_end+2*length_period*bpacking);
      int left_boundary_start = int(internal_index_end+4*length_period*bpacking+2*x_lenghth*bpacking);
      int left_boundary_end = int(internal_index_end+5*length_period*bpacking+2*x_lenghth*bpacking);
      int boundary_periodic_update_region = 30;// region means index

      // right boundary start and end index
      for (int i =right_boundary_start-boundary_periodic_update_region; i < right_boundary_start+boundary_periodic_update_region; i++)
      {
          int pi = particles[i];
          Particle& p = m_system->get_particle(pi);

          if (p.y<=length_period/2.0)
          {
              right_boundary_start=i;
              break;
          }
      }
      for (int i =right_boundary_end-boundary_periodic_update_region; i < right_boundary_end+boundary_periodic_update_region; i++)
      {
          int pi = particles[i];
          Particle& p = m_system->get_particle(pi);

          if (p.y<=-length_period/2.0)
          {
              right_boundary_end=i;
              break;
          }
      }
      Particle& p1 = m_system->get_particle(particles[right_boundary_start]);
      Particle& p2 = m_system->get_particle(particles[right_boundary_end]);
      if (abs(p1.y-p2.y)<0.001)
          right_boundary_end -= 1;
      // left boundary start and end index
      for (int i =left_boundary_start-boundary_periodic_update_region; i < left_boundary_start+boundary_periodic_update_region; i++)
      {
          int pi = particles[i];
          Particle& p = m_system->get_particle(pi);

          if (p.y>=-length_period/2.0)
          {
              left_boundary_start=i;
              break;
          }
      }
      for (int i =left_boundary_end-boundary_periodic_update_region; i < left_boundary_end+boundary_periodic_update_region; i++)
      {
          int pi = particles[i];
          Particle& p = m_system->get_particle(pi);

          if (p.y>=length_period/2.0)
          {
              left_boundary_end=i;
              break;
          }
      }
      Particle& p3 = m_system->get_particle(particles[left_boundary_start]);
      Particle& p4 = m_system->get_particle(particles[left_boundary_end]);
      if (abs(p4.y-p3.y)<0.001)
          left_boundary_end -= 1;

      // boundary periodic update region start and end index
      int right_up_boundary_end= right_boundary_start+1;
      int right_up_boundary_start = right_up_boundary_end-boundary_periodic_update_region+1;
      int right_down_boundary_start= right_boundary_end+1;
      int right_down_boundary_end = right_down_boundary_start+boundary_periodic_update_region-1;

      int left_up_boundary_start = left_boundary_end+1;
      int left_up_boundary_end = left_up_boundary_start+boundary_periodic_update_region-1;
      int left_down_boundary_end = left_boundary_start-1;
      int left_down_boundary_start = left_down_boundary_end-boundary_periodic_update_region+1;

      // periodic Update
      for (int i = up_internal_index_start; i < up_internal_index_end+1; i++)
      {
          int pi0 = particles[i-(up_internal_index_start-internal_index_start)];
          Particle& p0 = m_system->get_particle(pi0);
          int pi = particles[i];
          Particle& p = m_system->get_particle(pi);

          p.x = p0.x;
          p.y = p0.y+length_period;
          p.z = p0.z;

      }

      for (int i = down_internal_index_start; i < down_internal_index_end+1; i++)
      {
          int pi0 = particles[i-(down_internal_index_start-internal_index_start)];
          Particle& p0 = m_system->get_particle(pi0);
          int pi = particles[i];
          Particle& p = m_system->get_particle(pi);

          p.x = p0.x;
          p.y = p0.y-length_period;
          p.z = p0.z;

      }
        if (update_boundary)
        {
            // right boundary
            for (int i = right_up_boundary_start; i < right_up_boundary_end + 1; i++)
            {
                int pi0 = particles[i + (right_boundary_end - right_boundary_start + 1)];
                Particle& p0 = m_system->get_particle(pi0);
                int pi = particles[i];
                Particle& p = m_system->get_particle(pi);

                p.x = p0.x;
                p.y = p0.y + length_period;
                p.z = p0.z;

            }
            for (int i = right_down_boundary_start; i < right_down_boundary_end + 1; i++)
            {
                int pi0 = particles[i - (right_boundary_end - right_boundary_start + 1)];
                Particle& p0 = m_system->get_particle(pi0);
                int pi = particles[i];
                Particle& p = m_system->get_particle(pi);

                p.x = p0.x;
                p.y = p0.y - length_period;
                p.z = p0.z;

            }
            // left boundary
            for (int i = left_up_boundary_start; i < left_up_boundary_end + 1; i++)
            {
                int pi0 = particles[i - (left_boundary_end - left_boundary_start + 1)];
                Particle& p0 = m_system->get_particle(pi0);
                int pi = particles[i];
                Particle& p = m_system->get_particle(pi);

                p.x = p0.x;
                p.y = p0.y + length_period;
                p.z = p0.z;

            }
            for (int i = left_down_boundary_start; i < left_down_boundary_end + 1; i++)
            {
                int pi0 = particles[i + (left_boundary_end - left_boundary_start + 1)];
                Particle& p0 = m_system->get_particle(pi0);
                int pi = particles[i];
                Particle& p = m_system->get_particle(pi);

                p.x = p0.x;
                p.y = p0.y - length_period;
                p.z = p0.z;

            }
        }
  }
  // End of my changes
  // Update vertex mesh
  m_system->update_mesh();
}
