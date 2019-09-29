#include "Ipopt_MPC.h"
#include <math.h>
#include <vector>
#include "Eigen-3.3/Eigen/Core"
#include "IpIpoptApplication.hpp"


/** A bunch of global variables **/
//@{

/**
 * Set N and dt
 */
size_t N = 50;
double dt = 0.02;

// This value assumes the model presented in the classroom is used.
//
// It was obtained by measuring the radius formed by running the vehicle in the
// simulator around in a circle with a constant steering angle and velocity on a
// flat terrain.
//
// Lf was tuned until the the radius formed by the simulating the model
// presented in the classroom matched the previous radius.
//
// This is the length from front to CoG that has a similar radius.
const double Lf = 2.67;

// NOTE: feel free to play around with this or do something completely different
double ref_v = 20;

// The solver takes all the state variables and actuator
// variables in a singular vector. Thus, we should to establish
// when one variable starts and another ends to make our lifes easier.
size_t x_start = 0;
size_t y_start = x_start + N;
size_t psi_start = y_start + N;
size_t v_start = psi_start + N;
size_t delta_start = v_start + N;
size_t a_start = delta_start + N - 1;

//@}

// constructor
IpoptMPC::IpoptMPC() {}

// destructor
IpoptMPC::~IpoptMPC() {}

// return the size of the problem
bool IpoptMPC::get_nlp_info(Ipopt::Index& n, Ipopt::Index& m, Ipopt::Index& nnz_jac_g,
                            Ipopt::Index& nnz_h_lag, TNLP::IndexStyleEnum& index_style)
{
  // The problem described in Ipopt_MPC.hpp has ( 4 * N + 2 * (N - 1) ) variables
  n = 6 * N - 2;

  // The problem described in Ipopt_MPC.hpp has ( 4 * (N - 1) ) EUQALITY (state function) constraints 
  // and ( 2 * (N - 1) + 6 * N ) INEQUALITY (state/actuator) boundaries
  m = 4 * (N - 1);

  // non-zeros in the jacobian example
  nnz_jac_g = N; // ???

  // related to hessian
  nnz_h_lag = N; // ???

  // use the C style indexing (0-based)
  index_style = TNLP::C_STYLE;

  return true;
}

bool IpoptMPC::get_bounds_info(Ipopt::Index n, Ipopt::Number* x_l, Ipopt::Number* x_u,
                               Ipopt::Index m, Ipopt::Number* g_l, Ipopt::Number* g_u)
{
  // here we need to assert to make sure the variable numbers and constraint numbers
  // are what we think they are
  assert(n == 6 * N - 2);
  assert(m == 4 * N - 4);

  /* lower bound & upper bound */
  // Part I:
  // set all non-actuators upper and lower limits
  // to the max negative and positive values
  for (Ipopt::Index i = x_start; i < delta_start; i++)
  {
    x_l[i] = -1.0e19;
    x_u[i] = 1.0e19;
  }
  // Part II:
  // set all actuators upper and lower limits
  // according to their physics properties
  for (Ipopt::Index i = delta_start; i < a_start; i++)
  {
    x_l[i] = -0.436332;
    x_u[i] = 0.436332;
  }
  for (Ipopt::Index i = a_start; i < n; i++) // n: # of variables
  {
    x_l[i] = -1.0;
    x_u[i] = 1.0;
  }

  // the first constraint g1 has a lower bound of 25
  g_l[0] = 25; // ???
  // the first constraint g1 has NO upper bound, here we set it to 2e19.
  // Ipopt interprets any number greater than nlp_upper_bound_inf as
  // infinity. The default value of nlp_upper_bound_inf and nlp_lower_bound_inf
  // is 1e19 and can be changed through ipopt options.
  g_u[0] = 2e19; // ???

  // the second constraint g2 is an equality constraint, so we set the
  // upper and lower bound to the same value
  g_l[1] = g_u[1] = 40.0; // ???

  return true;
}

// return the initial point for the problem
bool IpoptMPC::get_starting_point(Ipopt::Index n, bool init_x, Ipopt::Number* x,
                                  bool init_z, Ipopt::Number* z_L, Ipopt::Number* z_U,
                                  Ipopt::Index m, bool init_lambda,
                                  Ipopt::Number* lambda)
{
  // Here, we assume we only have starting values for x, if you code
  // your own NLP, you can provide starting values for the dual variables
  // if you wish
  assert(init_x == true);
  assert(init_z == false);
  assert(init_lambda == false);

  // initialize to the given starting point
  /**
   * [Simple Reference]
   * x[0] = 1.0;
   * x[1] = 5.0;
   * x[2] = 5.0;
   * x[3] = 1.0;
   */
  // Initial value of the idependent variables
  // Should be 0 except for the initial values
  for (int i = 0; i < n; i++)
  {
    x[i] = 0.0;
  }
  // Set the initial variable values
  x[x_start]   = x0_[0];
  x[y_start]   = x0_[1];
  x[psi_start] = x0_[2];
  x[v_start]   = x0_[3];

  return true;
}

// returns the value of the objective function
bool IpoptMPC::eval_f(Ipopt::Index n, const Ipopt::Number* x, bool new_x, Ipopt::Number& obj_value)
{
  assert(n == 6 * N - 2); // ???

  /**
   * [Simple Reference]
   * obj_value = x[0] * x[3] * (x[0] + x[1] + x[2]) + x[2];
   */

  return true;
}

// return the gradient of the objective function grad_{x} f(x)
bool IpoptMPC::eval_grad_f(Ipopt::Index n, const Ipopt::Number* x, bool new_x, Ipopt::Number* grad_f)
{ 
  // assert variable numbers
  assert(n == 6 * N - 2); // ???

  /**
   * [Simple Reference]
   * grad_f[0] = x[0] * x[3] + x[3] * (x[0] + x[1] + x[2]);
   * grad_f[1] = x[0] * x[3];
   * grad_f[2] = x[0] * x[3] + 1;
   * grad_f[3] = x[0] * (x[0] + x[1] + x[2]);
   */

  return true;
}

// return the value of the constraints: g(x)
bool IpoptMPC::eval_g(Ipopt::Index n, const Ipopt::Number* x, bool new_x, Ipopt::Index m, Ipopt::Number* g)
{
  assert(n == 6 * N - 2); // ???
  assert(m == 6 * N - 6); // ???

  /**
   *  [Simple Reference]
   *  g[0] = x[0] * x[1] * x[2] * x[3];
   *  g[1] = x[0]*x[0] + x[1]*x[1] + x[2]*x[2] + x[3]*x[3];
   */

  return true;
}

// return the sturcture or values of the jacobian
bool IpoptMPC::eval_jac_g(Ipopt::Index n, const Ipopt::Number* x, bool new_x,
                          Ipopt::Index m, Ipopt::Index nele_jac, Ipopt::Index* iRow, Ipopt::Index *jCol,
                          Ipopt::Number* values)
{
  // [Simple Reference]
  /* if (values == NULL) {
    // return the structure of the jacobian

    // this particular jacobian is dense
    iRow[0] = 0;
    jCol[0] = 0;
    iRow[1] = 0;
    jCol[1] = 1;
    iRow[2] = 0;
    jCol[2] = 2;
    iRow[3] = 0;
    jCol[3] = 3;
    iRow[4] = 1;
    jCol[4] = 0;
    iRow[5] = 1;
    jCol[5] = 1;
    iRow[6] = 1;
    jCol[6] = 2;
    iRow[7] = 1;
    jCol[7] = 3;
  }
  else {
    // return the values of the jacobian of the constraints

    values[0] = x[1]*x[2]*x[3]; // 0,0
    values[1] = x[0]*x[2]*x[3]; // 0,1
    values[2] = x[0]*x[1]*x[3]; // 0,2
    values[3] = x[0]*x[1]*x[2]; // 0,3

    values[4] = 2*x[0]; // 1,0
    values[5] = 2*x[1]; // 1,1
    values[6] = 2*x[2]; // 1,2
    values[7] = 2*x[3]; // 1,3
  } */

  return true;
}

//return the structure or values of the hessian
bool IpoptMPC::eval_h(Ipopt::Index n, const Ipopt::Number* x, bool new_x,
                      Ipopt::Number obj_factor, Ipopt::Index m, const Ipopt::Number* lambda,
                      bool new_lambda, Ipopt::Index nele_hess, Ipopt::Index* iRow,
                      Ipopt::Index* jCol, Ipopt::Number* values)
{
  // [Simple Reference]
  /* if (values == NULL) {
    // return the structure. This is a symmetric matrix, fill the lower left
    // triangle only.

    // the hessian for this problem is actually dense
    Ipopt::Index idx=0;
    for (Ipopt::Index row = 0; row < 4; row++) {
      for (Ipopt::Index col = 0; col <= row; col++) {
        iRow[idx] = row;
        jCol[idx] = col;
        idx++;
      }
    }

    assert(idx == nele_hess);
  }
  else {
    // return the values. This is a symmetric matrix, fill the lower left
    // triangle only

    // fill the objective portion
    values[0] = obj_factor * (2*x[3]); // 0,0

    values[1] = obj_factor * (x[3]);   // 1,0
    values[2] = 0.;                    // 1,1

    values[3] = obj_factor * (x[3]);   // 2,0
    values[4] = 0.;                    // 2,1
    values[5] = 0.;                    // 2,2

    values[6] = obj_factor * (2*x[0] + x[1] + x[2]); // 3,0
    values[7] = obj_factor * (x[0]);                 // 3,1
    values[8] = obj_factor * (x[0]);                 // 3,2
    values[9] = 0.;                                  // 3,3


    // add the portion for the first constraint
    values[1] += lambda[0] * (x[2] * x[3]); // 1,0

    values[3] += lambda[0] * (x[1] * x[3]); // 2,0
    values[4] += lambda[0] * (x[0] * x[3]); // 2,1

    values[6] += lambda[0] * (x[1] * x[2]); // 3,0
    values[7] += lambda[0] * (x[0] * x[2]); // 3,1
    values[8] += lambda[0] * (x[0] * x[1]); // 3,2

    // add the portion for the second constraint
    values[0] += lambda[1] * 2; // 0,0

    values[2] += lambda[1] * 2; // 1,1

    values[5] += lambda[1] * 2; // 2,2

    values[9] += lambda[1] * 2; // 3,3
  } */

  return true;
}

void IpoptMPC::finalize_solution(Ipopt::SolverReturn status,
                                 Ipopt::Index n, const Ipopt::Number* x, const Ipopt::Number* z_L, const Ipopt::Number* z_U,
                                 Ipopt::Index m, const Ipopt::Number* g, const Ipopt::Number* lambda,
                                 Ipopt::Number obj_value,
                                 const Ipopt::IpoptData* ip_data,
                                 Ipopt::IpoptCalculatedQuantities* ip_cq)
{
  // here is where we would store the solution to variables, or write to a file, etc
  // so we could use the solution.

  // [Simple reference]
  // For this example, we write the solution to the console
  /* std::cout << std::endl << std::endl << "Solution of the primal variables, x" << std::endl;
  for (Ipopt::Index i=0; i<n; i++) {
     std::cout << "x[" << i << "] = " << x[i] << std::endl;
  }

  std::cout << std::endl << std::endl << "Solution of the bound multipliers, z_L and z_U" << std::endl;
  for (Ipopt::Index i=0; i<n; i++) {
    std::cout << "z_L[" << i << "] = " << z_L[i] << std::endl;
  }
  for (Ipopt::Index i=0; i<n; i++) {
    std::cout << "z_U[" << i << "] = " << z_U[i] << std::endl;
  }

  std::cout << std::endl << std::endl << "Objective value" << std::endl;
  std::cout << "f(x*) = " << obj_value << std::endl;

  std::cout << std::endl << "Final value of the constraints:" << std::endl;
  for (Ipopt::Index i=0; i<m ;i++) {
    std::cout << "g(" << i << ") = " << g[i] << std::endl;
  } */
}