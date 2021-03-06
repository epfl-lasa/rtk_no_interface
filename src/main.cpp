
#include "signal.h"

#include "LWRCore_no.h"
#include "StdTools/XmlTree.h"
#include "StdTools/Various.h"
#include <vector>
#include <QApplication>

#include "ros/package.h"

using namespace std;

LWRCore *myCoreInterface;

vector<Robot *> myRobots;
vector<Robot *> mySimRobots;
vector<RobotInterface *> myRobotInterfaces;
World *myWorld = NULL;
vector<WorldInterface *> myWorldInterfaces;

boost::thread *mKUKAThread;

void sighandle(int signal)
{
  myCoreInterface->Stop();
  myCoreInterface->Free();
  exit(1);
}

void KUKAUpdate(void)
{
  int cnt = 0;

  // control loop is here

  ros::Rate r(1000.0);

  while (true)
  {
//    ROS_ERROR("KUKAUpdate");
    myCoreInterface->Update();
    cnt = 0;
    cnt++;
    myCoreInterface->UpdateCore();

    //        ROS_ERROR("KUKAUpdate end");

    r.sleep();

    //        ROS_ERROR("KUKAUpdate end after sleep");
  }
}

Robot *myRobot;
int main(int argc, char **argv)
{
  signal(SIGINT, sighandle);
  FileFinder::AddBasePath(".");
  FileFinder::AddBasePath("./data");

  std::string path = ros::package::getPath("rtk_pkg_tools");

  // Wow, so much skill.
  FileFinder::AddBasePath(path + "/..");
  FileFinder::AddBasePath(path + "/../data");

  XmlTree  args;
  pXmlTree tree, tree2;
  pXmlTreeList tlist, tlist2;
  char   modName[256];
  Robot *cRobot;
  vector<string> nameAndPatches;
  bool bShowHelp = false;
  bool bError    = false;

  XmlTree argStruct("args", "", 7, new XmlTree("config",    "", "needArg=\"true\""),
                    new XmlTree("debug",     "d", "needArg=\"\""),
                    new XmlTree("robot",     "r", "needArg=\"true\""),
                    new XmlTree("module",    "m", "needArg=\"true\""),
                    new XmlTree("args",      "a", "needArg=\"true\""),
                    new XmlTree("world",     "w", "needArg=\"true\""),
                    new XmlTree("help",      "h", "needArg=\"\"")
                    );

  bShowHelp = !args.ParseArguments(argc, argv, &argStruct);

  string  configFile;
  XmlTree config("Config");

  if ( ( tree = args.Find("config") ) != NULL ) {
    char txt[256];
    sprintf( txt, "config/%s.xml", tree->GetData().c_str() );

    if ( FileFinder::Find(txt) ) {
      if ( !config.LoadFromFile( FileFinder::GetCStr() ) ) {
        fprintf( stderr, "Error while reading file: %s\n", FileFinder::GetCStr() );
        fprintf(stderr, "Exiting\n");
        return -1;
      } else {
        fprintf( stderr, "Loading config file: %s\n", FileFinder::GetCStr() );
        configFile = FileFinder::GetString();
      }
    } else {
      fprintf(stderr, "Error: file %s for found\n", txt);
      fprintf(stderr, "Exiting\n");
      return -1;
    }
  }

  // Parsing arguments
  if ( args.Find("debug") ) {
    config.Set( "Debug", string("") );
  }

  if ( args.Find("robot") ) {
    while ( ( tree = config.Find("Robot") ) != NULL ) {
      config.DelSubTree(tree);
    }
    config.Set( "Robot", args.Get( "robot", string("") ) );
  }

  if ( args.Find("module") ) {
    if ( ( tree = config.Find("Robot") ) == NULL ) {
      cerr << "Error: not robot defined..." << endl;
      bShowHelp = true;
    } else {
      while ( ( tree2 = config.Find("RobotModule") ) != NULL ) {
        tree->DelSubTree(tree2);
      }
      tree->Set( "RobotModule", args.Get( "module", string("") ) );
    }
  }

  if ( args.Find("args") ) {
    if ( ( tree = config.Find("Robot") ) == NULL ) {
      cerr << "Error: not robot defined..." << endl;
      bShowHelp = true;
    } else {
      while ( ( tree2 = config.Find("Args") ) != NULL ) {
        tree->DelSubTree(tree2);
      }
      tree->Set( "Args", args.Get( "args", string("") ) );
    }
  }

  if ( args.Find("world") ) {
    while ( ( tree = config.Find("World") ) != NULL ) {
      config.DelSubTree(tree);
    }
    config.Set( "World", args.Get( "world", string("") ) );
  }

  std::string path_rtkpkgtools = ros::package::getPath("rtk_pkg_tools");

  bool bHasInterfacePath = false;

  if ( ( tree = config.Find("Packages") ) != NULL ) {
    vector<string> paths = Tokenize( config.Find("Packages")->GetData() );
    int cnt              = 0;

    for (unsigned int i = 0; i < paths.size(); i++) {
      if (paths[i] == "LWRInterface") bHasInterfacePath = true;

      string path = string("./data/packages/") + paths[i];
      paths[i] = path;

      if (path.length() > 0) {
        FileFinder::AddAdditionalPath(path);

        // Wow, such hack.
        FileFinder::AddAdditionalPath(path_rtkpkgtools + "/." + path);
        cnt++;
      }
    }

    if (cnt > 0) {
      gLOG.AppendToEntry( "Messages", "Using package path(s): <%s>", Serialize(paths).c_str() );
    }
  }

  if (!bHasInterfacePath) {
    string path = string("./data/packages/LWRInterface");
    FileFinder::AddAdditionalPath(path);

    // Wow, such hack.
    FileFinder::AddAdditionalPath(path_rtkpkgtools + "/." + path);

    gLOG.AppendToEntry( "Messages", "Adding package path: <%s>", path.c_str() );
    bHasInterfacePath = true;
  }


  myCoreInterface = new LWRCore();

  myCoreInterface->SetName("KUKA");
  myCoreInterface->GetConsole()->SetName("KUKA");


  myWorld = new World();

  if ( ( tree = config.Find("World") ) != NULL ) {
    if (tree->GetData().length() > 0) {
      if ( !myWorld->Load( tree->GetData() ) ) {
        gLOG.AppendToEntry( "Messages", "Error while opening and reading world file: %s", tree->GetData().c_str() );
        bError = true;

        //                Cleanup();
        return false;
      }
    }


    // World modules
    tlist = tree->GetSubTrees();

    for (int i = 0; i < int( tlist->size() ); i++) {
      if (tlist->at(i)->GetName() == "WorldModule") {
        sprintf( modName, "module/%s.so", tlist->at(i)->GetData().c_str() );

        if ( FileFinder::Find(modName) ) {
          ModuleInterface *interface = ModuleInterface::LoadInterface( FileFinder::GetCStr() );

          if (dynamic_cast<WorldInterface *>(interface) != NULL) {
            interface->SetOptionTree( tlist->at(i) );
            interface->SetAutoPerformanceTiming(false);
            myWorldInterfaces.push_back( (WorldInterface *)interface );
            myWorld->AddInterface( (WorldInterface *)interface );

            if (tlist->at(i)->Get( "Name", string("") ).length() > 0) {
              interface->SetName( string("World_") + tlist->at(i)->Get( "Name", string("") ) );
            } else {
              interface->SetName( string("World") );
            }

            //                        myCoreInterface->GetConsole()->AddConsole(interface->GetConsole());
            gLOG.AppendToEntry("Messages", "World module <%s> sucessfully loaded", modName);
          } else if (interface != NULL) {
            ModuleInterface::CloseInterface(interface);
            gLOG.AppendToEntry("Messages", "Error: Module <%s> is not a world module", modName);
            bError = true;
          } else {
            gLOG.AppendToEntry("Messages", "Error: Failed to load world module <%s>", modName);
            bError = true;
          }
        } else {
          gLOG.AppendToEntry("Messages", "Error: Failed to find world module <%s>", modName);
          bError = true;
        }
      }
    }
  }


  if (bError) {
    return 0;
  }


  // Loading robots
  tlist = config.GetSubTrees();

  for (int i = 0; i < int( tlist->size() ); i++) {
    if (tlist->at(i)->GetName() == "Robot") {
      tree           = tlist->at(i);
      nameAndPatches = Tokenize( RemoveSpaces( tree->GetData() ) );

      if (nameAndPatches.size() > 0) {
        if ( (nameAndPatches[0].substr(0, 4) == "KUKA") || (nameAndPatches[0].substr(0, 3) == "LWR") ) {
          cRobot = new LWRRobot();
        }
        else cRobot = new Robot();

        if ( !cRobot->Load( nameAndPatches[0], Serialize(nameAndPatches, 1) ) ) {
          gLOG.AppendToEntry( "Messages", "Error: While operning and reading robot file <%s>", nameAndPatches[0].c_str() );
          delete cRobot; cRobot = NULL;
          bError                = true;
        } else {
          cRobot->SetArgs( Tokenize( tree->Get( "Args", string("") ) ) );

          // cRobot->SetName(tree->CGet("Name",string("Unknown")));
          cRobot->SetName( tree->CGet( "Name", cRobot->GetType() ) );
          cRobot->SetWorld(myWorld);
          myRobots.push_back(cRobot);

          tlist2 = tree->GetSubTrees();

          for (int j = 0; j < int( tlist2->size() ); j++) {
            if (tlist2->at(j)->GetName() == "RobotModule") {
              sprintf( modName, "module/%s.so", tlist2->at(j)->GetData().c_str() );

              if ( FileFinder::Find(modName) ) {
                ModuleInterface *interface = ModuleInterface::LoadInterface( FileFinder::GetCStr() );

                if (dynamic_cast<RobotInterface *>(interface) != NULL) {
                  interface->SetOptionTree( tlist2->at(j) );
                  interface->SetAutoPerformanceTiming(false);
                  myRobotInterfaces.push_back( (RobotInterface *)interface );
                  cRobot->AddInterface( (RobotInterface *)interface );

                  if (tlist2->at(j)->Get( "Name", string("") ).length() > 0) {
                    interface->SetName( cRobot->GetName() + string("_") + tlist2->at(j)->Get( "Name", string("") ) );
                  } else {
                    interface->SetName( cRobot->GetName() );
                  }

                  //                                    myCoreInterface->GetConsole()->AddConsole(interface->GetConsole());
                  gLOG.AppendToEntry("Messages", "Robot module <%s> sucessfully loaded", modName);
                } else if (interface != NULL) {
                  ModuleInterface::CloseInterface(interface);
                  gLOG.AppendToEntry("Messages", "Error: Module <%s> is not a robot module", modName);
                  bError = true;
                } else {
                  gLOG.AppendToEntry("Messages", "Error: Failed to load robot module <%s>", modName);
                  bError = true;
                }
              } else {
                gLOG.AppendToEntry("Messages", "Error: Failed to find robot module <%s>", modName);
                bError = true;
              }
            }
          }

          // Setting up robot object and put it into the world
          WorldObject *wObject = new WorldObject();
          wObject->SetName( cRobot->GetName() );
          wObject->SetRobot(cRobot);

          if ( tree->Find("Origin") ) {
            int size;
            REALTYPE *array;
            size = tree->GetArray("Origin", &array);

            if (size == 3) {
              wObject->GetReferenceFrame(true).SetOrigin().Set(array);
            } else {
              cerr <<  "Error: Bad robot <Origin> array size (should be 3)" << endl;
            }
          }

          if ( tree->Find("Orient") ) {
            int size;
            double *array;
            size = tree->GetArray("Orient", &array);

            if (size == 9) {
              wObject->GetReferenceFrame(true).SetOrient().Set(array);
              wObject->GetReferenceFrame(true).SetOrient().Normalize();
            } else if (size == 3) {
              wObject->GetReferenceFrame(true).SetOrient().SRotationV( Vector3(array) );
            } else {
              cerr << "Error: Bad robot <Orient> array size (should be 3(axis*angle) or 9(full rotation matrix))" << endl;
            }
          }
          wObject->SetToInitialState();
          wObject->SetInitialState();

          myWorld->AddObject(wObject, true);
        }
      } else {
        cerr << "No robot type defined." << endl;
        bError = true;
      }
    }
  }


  if (bError) {
    return 0;
  }


  LWRRobot *myLWR = NULL;

  for (int i = 0; i < myRobots.size(); i++) {
    cout << "the robot type" << endl;
    cout << myRobots[i]->GetType() << endl;

    if ( (myRobots[i]->GetType().substr(0, 3) == "LWR") || (myRobots[i]->GetType().substr(0, 4) == "KUKA") ) {
      myLWR = (LWRRobot *)myRobots[i];
      break;
    }
  }


  if (myLWR == NULL) {
    cerr << "Error: No KUKA robot defined." << endl;

    //        Cleanup();
    return 0;
  }

  cout << "my LWR robot has: " << myLWR->GetLinksCount() << " links " << endl;
  cout << "my LWR robot has: " << myLWR->GetActuatorsCount() << " actuators " << endl;
  cout << "my LWR robot has: " << myLWR->GetSensorsCount() << " sensors " << endl;


  myCoreInterface->SetRobot(myLWR);
  myCoreInterface->SetWorld(myWorld);


  if (myCoreInterface->Init() != RobotInterface::STATUS_OK)
  {
    myCoreInterface->Stop();
    myCoreInterface->Free();
    cout << "Cannot init" << endl;
    exit(1);
  }

  // ok

  // the LWRCore starts a console thread here
  if (myCoreInterface->Start() != RobotInterface::STATUS_OK)
  {
    myCoreInterface->Stop();
    myCoreInterface->Free();
    cout << "Cannot start" << endl;
    return 0;
  }

  KUKAUpdate();

  return 0;
}
