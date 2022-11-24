#include "../Game/EngineConfig.h"

#if defined(_MSC_VER)
#	pragma warning(push, 0)
#	pragma warning(disable : 4702)
#	pragma warning(disable : 5045)
#	pragma warning(disable : 5264)
#endif

#if USE_BULLET
// BulletDynamics
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp"
#include "BulletDynamics/Dynamics/btRigidBody.cpp"
#include "BulletDynamics/Dynamics/btSimulationIslandManagerMt.cpp"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.cpp"
#include "BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp"
#include "BulletDynamics/ConstraintSolver/btBatchedConstraints.cpp"
#include "BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btSliderConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btContactConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp"
#include "BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btFixedConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btHingeConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btGearConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.cpp"
#include "BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp"
#include "BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.cpp"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.cpp"
#include "BulletDynamics/MLCPSolvers/btDantzigLCP.cpp"
#include "BulletDynamics/MLCPSolvers/btLemkeAlgorithm.cpp"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.cpp"
#include "BulletDynamics/Featherstone/btMultiBody.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyJointMotor.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyGearConstraint.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyConstraint.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyFixedConstraint.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyPoint2Point.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyConstraintSolver.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyMLCPConstraintSolver.cpp"
#include "BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.cpp"
#include "BulletDynamics/Featherstone/btMultiBodySliderConstraint.cpp"
#include "BulletDynamics/Featherstone/btMultiBodySphericalJointMotor.cpp"
#include "BulletDynamics/Featherstone/btMultiBodySphericalJointLimit.cpp"
#include "BulletDynamics/Vehicle/btRaycastVehicle.cpp"
#include "BulletDynamics/Vehicle/btWheelInfo.cpp"
#include "BulletDynamics/Character/btKinematicCharacterController.cpp"
#endif

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif