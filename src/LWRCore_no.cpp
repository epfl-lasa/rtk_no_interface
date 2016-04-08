#include "LWRCore_no.h"


using namespace std;


LWRCore::LWRCore() : RobotInterface(true)
{
  // get the path to the LWRInterface package
}

LWRCore::~LWRCore()
{}

RobotInterface::Status LWRCore::RobotInit()
{
  AddConsoleCommand("quit");


  // setting up the sensorslist to point to the sensors of the robot object
  if (mRobot == NULL) {
    cout << "Error, no robot specified. Use .SetRobot() method before .Init" << endl;
  }
  else {
    mLWRRobot = (LWRRobot *)mRobot;

    int Nsens = mRobot->GetDOFCount();
    cout << "dof: " << Nsens << endl;
    cout << "am here" << endl;

    mJointSensors.resize(Nsens);
    mJointActuators.resize(Nsens);
  }

  //  SensorsUpdate();

  return STATUS_OK;
}

RobotInterface::Status LWRCore::RobotFree()
{
  return STATUS_OK;
}

RobotInterface::Status LWRCore::RobotStart()
{
  bRunConsoleLoop = true;

  // This will override the cout
  //  mStdout = cout.rdbuf();
  //  cout.rdbuf(mOutputStream.rdbuf());
  //  mNCConsole.SetConsole(GetConsole());
  //  mNCConsole.InitNCurses();
  //  mNCConsole.SetTopStaticLinesCount(7);

  //  mStdout = cout.rdbuf();
  //  cout.rdbuf(mOutputStream.rdbuf());
  //  mDisplayThread = boost::thread(&LWRCore::ConsoleLoop,  this);


  return STATUS_OK;
}

RobotInterface::Status LWRCore::RobotStop()
{
  bRunConsoleLoop = false;
  usleep(100000);

  //  mDisplayThread.join();

  return STATUS_OK;
}

RobotInterface::Status LWRCore::RobotUpdate() {
  return STATUS_OK;
}

RobotInterface::Status LWRCore::RobotUpdateCore() {
  return STATUS_OK;
}

int cnt = 0;
void LWRCore::ControlUpdate() {
  switch (nControl) {}
}

void LWRCore::ConsoleLoop() {
  //  while(bRunConsoleLoop == true){
  //    usleep(10000);			// 10 ms rate of refresh for commandline interface
  //    ConsoleUpdate();
  //  }
}

void LWRCore::ConsoleUpdate() {
  string s = mOutputStream.str();

  size_t cpos = 0;
  size_t opos = 0;

  if (s.size() > 0) {
    for (unsigned int i = 0; i < s.size(); i++) {
      opos = cpos;
      cpos = s.find("\n", opos);
      string ss = s.substr(opos, cpos - opos);

      if (ss.size() > 0) {
        mNCConsole.Print(ss);
      }

      if (cpos == string::npos) break;
      cpos++;
    }
    mOutputStream.str("");
  }
  mNCConsole.SetTopStaticLine(1,  "static_txt");

  mNCConsole.Process();
  mNCConsole.Render();
}

int LWRCore::RespondToConsoleCommand(const string command, const vector<string>& args)
{
  GetConsole()->ClearLine();

  if (command == "quit")
  {
    Stop();
    Free();
    exit(1);
  }


  return 0;
}
