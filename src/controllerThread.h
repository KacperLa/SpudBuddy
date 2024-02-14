#ifndef CONTROLLER_THREAD_H
#define CONTROLLER_THREAD_H

#include <ronThread.h>
#include <shared_structs.h>
#include <driveSystem.h>
#include<FSM.h>

using fsm_handle = Robot;

class controllerThread : public ronThread
{
public:
  controllerThread(const std::string name, Log* logger, driveSystem& drive_system);
  virtual ~controllerThread();
  
private:
  virtual void loop() override;
  void request_transition(int action);

  // create a SData writer for actual state of robot
  SData<systemActual_t> shared_actual_map;

protected:

  std::uint64_t  loop_time = (1000000000 / 1000); 
};

#endif
