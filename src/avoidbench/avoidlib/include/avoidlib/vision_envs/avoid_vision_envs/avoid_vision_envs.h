#pragma once

// std lib
#include <stdlib.h>

#include <cmath>
#include <iostream>

// yaml cpp
#include <yaml-cpp/yaml.h>

// avoidlib
#include "avoidlib/bridges/unity_bridge.hpp"
#include "avoidlib/common/logger.hpp"
#include "avoidlib/common/types.hpp"
#include "avoidlib/common/utils.hpp"
#include "avoidlib/common/math.hpp"
#include "avoidlib/objects/quadrotor.hpp"
#include "avoidlib/vision_envs/vision_env_base.h"
#include "avoidlib/sensors/rgb_camera.hpp"

namespace avoidlib {
namespace avoidenv {

enum Avoid : int {
  kNState = 14,
  kObs = 0,
  kNObs = 6,
  kNLatent = 32,
  kNSeq = 1,
  kAct = 0,
  kNAct = 4
};
}

class AvoidVisionEnv final : public VisionEnvBase {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  AvoidVisionEnv();
  AvoidVisionEnv(const std::string &cfg_path, const int env_id);
  AvoidVisionEnv(const YAML::Node &cfg_node, const int env_id);
  ~AvoidVisionEnv();

  // - public OpenAI-gym-style functions
  bool reset(Ref<Vector<>> obs) override;
  bool reset(Ref<Vector<>> obs, bool random);
  bool step(const Ref<Vector<>> act, Ref<Vector<>> obs,
            Ref<Vector<>> reward) override;

  // - public set functions
  bool loadParam(const YAML::Node &cfg);
  // - public get functions
  bool getObs(Ref<Vector<>> obs) override;
  bool getImage(Ref<ImgVector<>> img, const bool rgb = true) override;
  bool getDepthImage(Ref<DepthImgVector<>> img) override;

  // get states
  bool getQuadAct(Ref<Vector<>> act) const;
  bool getQuadState(Ref<Vector<>> state) const;
  bool getCollisionState() const;
  // - auxiliar functions
  bool isTerminalState(double &reward) override;
  bool addQuadrotorToUnity(const std::shared_ptr<UnityBridge> bridge) override;
  bool setPointClouds(const std::shared_ptr<Environment> env_ptr) override;
  void setQuadFromPtr(const std::shared_ptr<UnityBridge> bridge) override;

  friend std::ostream &operator<<(std::ostream &os,
                                  const AvoidVisionEnv &avoid_vision_env);
  inline std::vector<std::string> getRewardNames() {return reward_names_;}
  std::unordered_map<std::string, float> extra_info_;

 private:
  void init();
  int env_id_;
  int round;
  bool configCamera(const YAML::Node &cfg, const std::shared_ptr<RGBCamera>);
  Logger logger_{"QaudrotorEnv"};
  bool LineCollisionCheck(Vector<3> pt1, Vector<3> pt2);

  std::shared_ptr<Quadrotor> quad_ptr_;
  QuadState quad_state_, pre_quad_state_;
  Vector<3> goal_point_;
  Vector<3> direction_, pre_direction_;
  // Define reward for training
  double colli_coeff_, risk_coeff_, goal_coeff_, input_coeff_, lin_vel_coeff_;
  bool is_training;
  bool collision_happened;
  // observations and actions (for RL)
  Vector<avoidenv::kNObs> pi_obs_;
  Vector<avoidenv::kNAct> pi_act_;
  Vector<avoidenv::kNSeq*avoidenv::kNAct> pi_act_seq_;
  // reward function design (for model-free reinforcement learning)

  // action and observation normalization (for learning)
  Vector<avoidenv::kNAct> act_mean_;
  Vector<avoidenv::kNAct> act_std_;
  Vector<avoidenv::kNObs> obs_mean_ = Vector<avoidenv::kNObs>::Zero();
  Vector<avoidenv::kNObs> obs_std_ = Vector<avoidenv::kNObs>::Ones();

  // robot vision
  std::shared_ptr<RGBCamera> rgb_camera_;
  cv::Mat rgb_img_;
  cv::Mat depth_img_;

  // auxiliary variables
  bool use_camera_{true};
  YAML::Node cfg_;
  std::vector<std::string> reward_names_;
  Matrix<3, 2> world_box_;

  // point clouds map
  std::shared_ptr<Environment> env_ptr_;
};
}