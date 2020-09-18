#include "WipingController_WipeItBaby_rh.h"

#include "../WipingController.h"

void WipingController_WipeItBaby_rh::configure(const mc_rtc::Configuration & config)
{
	if(config.has("admittance"))
	{
	  admittance_ = config("admittance");
	}
  if(config.has("feetForceControl"))
  {
    feetForceControl_ = config("feetForceControl");
  }
}

void WipingController_WipeItBaby_rh::start(mc_control::fsm::Controller & ctl_)
{
  auto & ctl = static_cast<WipingController &>(ctl_);


  if(feetForceControl_)
  {
    ctl.addFootForceControl();
  }
  else
  {
    ctl.removeFootForceControl();
  }
  ctl.comTask->reset();
  ctl.comTask->weight(2000);
  ctl.comTask->stiffness(60);

  ctl.lookAtTask->stiffness(5);
  ctl.lookAtTask->weight(10);
  ctl.solver().addTask(ctl.lookAtTask);

  ctl.rightHandTask->reset();
  //ctl.rightHandTask->setGains(1, 300);
  //ctl.rightHandTask->stiffness(1.);
  //ctl.rightHandTask->damping(300.);
  Eigen::Vector6d dimW;
  dimW << 1., 1., 1., 0., 0., 1.;
  sva::MotionVecd stiffnessGain, dampingGain;
  stiffnessGain.angular() << 10, 10, 10;
  stiffnessGain.linear() << 10, 10, 5;
  dampingGain.angular() << 6, 6, 6;
  dampingGain.linear() << 6, 6, 300;
  ctl.rightHandTask->setGains(stiffnessGain, dampingGain);
  ctl.rightHandTask->dimWeight(dimW);
  ctl.rightHandTask->admittance(admittance_);
  ctl.rightHandTask->targetCoP(Eigen::Vector2d::Zero());
  ctl.rightHandTask->targetPose();
  ctl.setTargetFromCoMQP();
  ctl.addRightHandForceControl();

  //ctl.setFeetTargetFromCoMQP();
  //ctl.addLeftFootForceControl();
  ctl.solver().addTask(ctl.comTask);

  ctl.comQP().addToLogger(ctl.logger());
  //xx = 10.0;
  //ctl.logger().addLogEntry("EstimatedFriction",
  //                         []()
  //                         { double xxx = 100.0;
  //                         return xx;
  //                         });

  ctl.logger().addLogEntry("friction_mu_Estim",
                           [&ctl]()
                           {
                           return ctl.frictionEstimator.mu_calc();
                           });
  ctl.logger().addLogEntry("SurfaceWrench_x",
                           [&ctl]()
                           {
                           return ctl.frictionEstimator.forceX();
                           });
  ctl.logger().addLogEntry("SurfaceWrench_y",
                           [&ctl]()
                           {
                           return ctl.frictionEstimator.forceY();
                           });
  ctl.logger().addLogEntry("SurfaceWrench_z",
                           [&ctl]()
                           {
                           return ctl.frictionEstimator.forceZ();
                           });
  //ctl.logger().addLogEntry("friction_mu_y",
  //                         [&ctl]()
  //                         {
  //                         return ctl.frictionEstimator.mu_y();
  //                         });
  //ctl.logger().addLogEntry("friction_mu",
  //                         [&ctl]()
  //                         {
  //                         return ctl.frictionEstimator.mu();
  //                         });
}

bool WipingController_WipeItBaby_rh::run(mc_control::fsm::Controller & ctl_)
{
  auto & ctl = static_cast<WipingController &>(ctl_);
  Eigen::Vector3d delta;
  delta << 0.0003, 0.0, 0.0003; //This should generate straight line wiping on 45deg tilted_board
  sva::PTransformd poseOutput = ctl.rightHandTask->targetPose();
  auto rotationPose = poseOutput.rotation();
  auto translationPose = poseOutput.translation();
  Eigen::Vector6d rightHandPose_ = ctl.comQP().rh_pose();
  sva::PTransformd target;
  //target = poseOutput; //To keep the same rotation for hand
  //target.translation() = rightHandPose_.tail<3>() + delta; //To add value for translation of hand
  target.rotation() = rotationPose;
  target.translation() = translationPose + delta;
  ctl.rightHandTask->targetPose(target);
  //ctl.frictionEstimator.update(ctl.robot());
  //double EstimatedFriction = ctl.frictionEstimator.mu_calc();
  //mc_rtc::log::info("EstimatedFriction: {}", EstimatedFriction);
  ctl.setTargetFromCoMQP();
  //ctl.setFeetTargetFromCoMQP();
  // handForceFilter_.add(ctl.robot().forceSensor("RightHandForceSensor").worldWrench(ctl.robot()));

  output("OK");
  return true;
}

void WipingController_WipeItBaby_rh::teardown(mc_control::fsm::Controller & ctl_)
{
  auto & ctl = static_cast<WipingController &>(ctl_);
  ctl.removeRightHandForceControl();
  //ctl.removeLeftFootForceControl();
  ctl.removeFootForceControl();
  ctl.solver().removeTask(ctl.comTask);
  ctl.solver().removeTask(ctl.lookAtTask);

  ctl.lookAtTask->stiffness(1);
  ctl.lookAtTask->weight(1);

  ctl.comQP().removeFromLogger(ctl.logger());

  ctl.logger().removeLogEntry("friction_mu_x_Estim");
  //ctl.logger().removeLogEntry("friction_mu_y");
  //ctl.logger().removeLogEntry("friction_mu");

}

EXPORT_SINGLE_STATE("WipingController_WipeItBaby_rh", WipingController_WipeItBaby_rh)
