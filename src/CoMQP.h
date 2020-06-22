#pragma once

#include <mc_rbdyn/Robot.h>
#include <mc_rtc/Configuration.h>
#include <mc_rtc/gui.h>
#include <mc_rtc/log/Logger.h>

#include <Eigen/Dense>
#include <eigen-quadprog/QuadProg.h>

struct CoMQPResult
{
  Eigen::Vector3d comPos;
  sva::ForceVecd rightFootForce;
  sva::ForceVecd leftFootForce;
  sva::ForceVecd rightHandForce;
  sva::ForceVecd leftHandForce;
};

class CoMQP
{
public:
  CoMQP(const mc_rbdyn::Robot & robot, const mc_rtc::Configuration & config);
  bool solve(const mc_rbdyn::Robot & robot);
  //bool solve(const mc_rbdyn::Robot & robot, const ModifiedSupportPolygon & supportPolygon);
  /*!
   * @brief  Result expressed as
   * - CoM position [x,y,z]
   * - Right foot force [fx, fy, fz]
   * - Right foot torque [tau_x, tau_y, tau_z]
   * - Left foot force [fx, fy, fz]
   * - Left foot torque [tau_x, tau_y, tau_z]
   * - Right hand force [fx, fy, fz]
   * - Right hand torque [tau_x, tau_y, tau_z]
   * @returns QP result
   */
  const Eigen::VectorXd & resultVector() const;
  CoMQPResult result() const;

  int errorCode() const;
  void errorMessage() const;

  bool debug() const
  {
    return debug_;
  }

  double desiredNormalForce() const
  {
    return N;
  }

  void desiredNormalForce(double v)
  {
    N = v;
  }

  Eigen::Vector2d muXY() const
  {
    return {mu_x, mu_y};
  }

  void addToGUI(mc_rtc::gui::StateBuilder &);
  void removeFromGUI(mc_rtc::gui::StateBuilder &);
  void addToLogger(mc_rtc::Logger &);
  void removeFromLogger(mc_rtc::Logger &);

protected:
  int nrVar() const
  {
    return numVar_;
  }
  int nrEq() const
  {
    return A_st.rows();
  }
  int nrInEq() const
  {
    return G_st.rows();
  }

protected:
  bool debug_ = false;
  int numVar_ = 0;
  int numEq_ = 0;

  // Mass offset in kg (the real robot weight is different from the model)
  double massOffset_ = 0;
  double mg = 0;

  // Dimensions of the foot, hardcoded
  // XXX should probably use support polygon instead
  double X_rf;
  double Y_rf;
  double X_lf;
  double Y_lf;
  double X_rh;
  double Y_rh;

  double mu_rf, mu_lf, mu_rh;

  const double eps = -1e-4;

  // XXX use realistic limits here?
  const double fz_max = 8000;
  const double fz_min = -8000;
  const double k = 3; //number of contacts
  const double n = 1; //number of sliding contacts
  Eigen::Vector3d com_pos;

  Eigen::MatrixXd E_m1;
  Eigen::VectorXd E_m2;
  Eigen::MatrixXd E_lf, E_rf, E_rh, E_lh;
  Eigen::MatrixXd sliding1, sliding2, sliding, sliding_tmp, sliding2_tmp, sliding_tmp_tr, sliding1_check, sliding2_check;
  Eigen::MatrixXd A, A_st;
  Eigen::MatrixXd UBmat_ineq, UBmat_lf, UBmat_rf, UBmat_rh, UBmat_rh_tmp;
  Eigen::MatrixXd LBmat_ineq, LBmat_lf, LBmat_rf, LBmat_rh, LBmat_rh_tmp;
  Eigen::VectorXd b, b_st;

  Eigen::MatrixXd Ineq_mat1, Ineq_mat2, Ineq_mat3, Ineq_mat4, Ineq_mat5, Ineq_mat6;
  Eigen::MatrixXd Ineq_max_rf, Ineq_min_rf, Ineq_max_lf, Ineq_min_lf, Ineq_max_rh, Ineq_min_rh, Ineq_max_nonRotate;
  Eigen::MatrixXd G, G_st; // G_st is for G* matrix (star) 
  Eigen::VectorXd h, h_st;
  Eigen::MatrixXd P, P_st;
  Eigen::VectorXd Y_desired;
  Eigen::VectorXd q, q_st;
  Eigen::Vector3d PG_d;

  double P_PG = 10;
  double P_Force_rf = 1;
  double P_Wrench_rf = 1;
  double P_Force_lf = 1;
  double P_Wrench_lf = 1;
  double P_Force_rh = 10;
  double P_Wrench_rh = 10;
  double P_Radius = 0.1;

  // Desired normal force
  double N = 1;
  double mu_x = 0, mu_y = 0;

  std::string rightHandForceSensor = "RightHandForceSensor";
  std::string rightFootForceSensor = "RightFootForceSensor";
  std::string leftFootForceSensor = "LeftFootForceSensor";

  std::string rightHandSurface = "RightHandPad";
  std::string leftHandSurface = "LeftHand"; //XXX To add the correct element
  std::string rightFootSurface = "RightFoot";
  std::string leftFootSurface = "LeftFoot";

  Eigen::MatrixXd A_;
  Eigen::MatrixXd x_;
  Eigen::QuadProgDense solver_;
};
