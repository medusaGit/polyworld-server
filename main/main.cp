// System
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// STL
#include <string>

// Qt
#include <qgl.h>
#include <QApplication>
#include <QtCore>

// Local
#include "MainWindow.h"
#include "Monitor.h"
#include "Simulation.h"
#include "SimulationController.h"
#include "TerminalUI.h"

// Network
#include "NetworkDialog.h"
#include "MulticastSender.h"
#include "Server.h"

using namespace std;

//===========================================================================
// usage
//===========================================================================
void usage( const char* format, ... )
{
	printf( "Usage:  Polyworld [--ui gui|term] worldfile\n" );

	if( format )
	{
		printf( "Error:\n\t" );
		va_list argv;
		va_start( argv, format );
		vprintf( format, argv );
		va_end( argv );
		printf( "\n" );
	}

	exit( 1 );
}

void usage()
{
	usage( NULL );
}

//===========================================================================
// main
//===========================================================================
int main( int argc, char** argv )
{
	const char *worldfilePath = NULL;
	string ui = "gui";

	for( int argi = 1; argi < argc; argi++ )
	{
		string arg = argv[argi];

		if( arg[0] == '-' )	// it's a flagged argument
		{
			if( arg == "--ui" )
			{
				if( ++argi >= argc )
					usage( "Missing --ui arg" );

				ui = argv[argi];
				if( (ui != "gui") && (ui != "term") )
					usage( "Invalid --ui arg (%s)", argv[argi] );
			}
			else
				usage( "Unknown argument: %s", argv[argi] );
		}
		else
		{
			if( worldfilePath == NULL )
				worldfilePath = argv[argi];
			else
				usage( "Only one worldfile path allowed, at least two specified (%s, %s)", worldfilePath, argv[argi] );
		}
	}

	if( ! worldfilePath )
	{
		usage( "A valid path to a worldfile must be specified" );
	}

	string monitorPath;
	{
		if( exists("./" + ui + ".mf") )
			monitorPath = "./" + ui + ".mf";
		else
			monitorPath = "./etc/" + ui + ".mf";
	}

	// Make sure we're in an appropriate working directory
#if __linux__
	{
		char exe[1024];
		int rc = readlink( "/proc/self/exe", exe, sizeof(exe) );
		exe[rc] = 0;
		char *lastslash = strrchr( exe, '/' );
		*lastslash = 0;
		if( 0 != strcmp(exe, getenv("PWD")) )
		{
			fprintf( stderr, "Must execute from directory containing binary: %s\n", exe );
			exit( 1 );
		}
	}
#endif

	QApplication app(argc, argv);

    if (!QGLFormat::hasOpenGL())
    {
		qWarning("This system has no OpenGL support. Exiting.");
		return -1;
    }

	// Establish how our preference settings file will be named
	QCoreApplication::setOrganizationDomain( "indiana.edu" );
	QCoreApplication::setApplicationName( "polyworld" );

	// It is necessary to force the "C" locale because Qt adopts the system
	// locale, but we want floats specified with a period to work correctly
	// even in locales that normally use a comma for the decimal mark.
	setlocale( LC_NUMERIC, "C" );

    TSimulation *simulation = new TSimulation( worldfilePath, monitorPath );
    SimulationController *simulationController = new SimulationController( simulation );

    int exitval;

	if( ui == "gui" )
	{
        MulticastSender multicastSender;
        //multicastSender.show();

        // testing full TCP server
        Server server;

        // send the multicast step number over the network
        QObject::connect(simulation, SIGNAL(stepChanged(int, float)),
                         &server, SLOT(simStepMsg(int, float)));

        // Multicast Sender

        // send the multicast step number over the network
        QObject::connect(simulation, SIGNAL(stepChanged(int, float)),
                         &multicastSender, SLOT(simStepMsg(int, float)));

        // send the multicast agent birth over the network
        QObject::connect(simulation, SIGNAL(agentBirth(agent*)),
                         &multicastSender, SLOT(agentBirthMsg(agent*)));

        // send the multicast agent death over the network
        QObject::connect(simulation, SIGNAL(agentDeath(agent*)),
                         &multicastSender, SLOT(agentDeathMsg(agent*)));

        // send the multicast food add over the network
        QObject::connect(simulation, SIGNAL(foodAdded(food*)),
                         &multicastSender, SLOT(foodAddMsg(food*)));

        // send the multicast food add over the network
        QObject::connect(simulation, SIGNAL(foodRemoved(food*)),
                         &multicastSender, SLOT(foodRemoveMsg(food*)));

        // we HAVE to have this after the connectors are linked up so the birth signals get sent
        // initialize the world objects (ground/agents/bricks)
        simulation->InitWorld();

        MainWindow *mainWindow = new MainWindow( simulationController );
        simulationController->start();

        // network code
        NetworkDialog networkDialog;
        //networkDialog.show();
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

        exitval = app.exec();
        delete mainWindow;
	}
	else if( ui == "term" )
	{
        simulation->InitWorld();

        TerminalUI *term = new TerminalUI( simulationController );
		simulationController->start();
		exitval = app.exec();
        delete term;
	}
	else
		assert( false );

    delete simulationController;
    delete simulation;

	return exitval;
}
