#ifndef PRECICEADAPTER_H
#define PRECICEADAPTER_H

#include "Interface.H"

// Conjugate Heat Transfer module
#include "CHT/CHT.H"

// NOTE: If you want to couple a new variable, include your module's header here.
// You also need to include it in the Make/files file.
// In case you use additional OpenFOAM symbols, you may also need to specify
// the respective libraries in the Make/options.

// OpenFOAM header files
#include "Time.H"
#include "fvMesh.H"

// Other header files
// YAML reader - Used to read the adapter's configuration file.
#include "yaml-cpp/yaml.h"
// preCICE Solver Interface
#include "SolverInterface.hpp"

namespace preciceAdapter
{

class Adapter
{

private:

    //- Structure of the configuration of each coupling interface.
    //  Every interface needs to know the coupling mesh, the OpenFOAM
    //  patches that consist the coupling surface and the kinds
    //  of data that are exchanged.
    struct InterfaceConfig
    {
        std::string meshName;
        std::vector<std::string> patchNames;
        std::vector<std::string> writeData;
        std::vector<std::string> readData;
    };

    //- Configuration interfaces
    std::vector<struct InterfaceConfig> interfacesConfig_;

    //- OpenFOAM runTime object
    const Foam::Time& runTime_;

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // Configuration parameters used in the Adapter

        //- Remember if there were errors in the read() method
        bool errorsInConfigure = false;

        //- preCICE participant name
        std::string participantName_;

        //- preCICE configuration file name
        std::string preciceConfigFilename_;

        //- Switch for subcycling
        bool subcyclingAllowed_ = true;

        //- Switch for checkpointing
        bool checkpointing_ = false;

        //- Switch for automatically preventing the solver from exiting before
        //  the coupling is complete.
        bool preventEarlyExit_ = true;

        //- Switch for evaluating the boundaries after reading a checkpoint.
        bool evaluateBoundaries_ = true;

        //- Switch to disable the checkpointing
        bool disableCheckpointing_ = false;

        //- Type of timestep (fixed, adjustable)
        bool adjustableTimestep_;

        //- Should the (fixed) timestep be stored and used?
        bool useStoredTimestep_ = false;

        //- Switch to enable the ConjugateHeatTransfer module
        bool CHTenabled_ = true;

    //- Interfaces
    std::vector<Interface*> interfaces_;

    //- preCICE solver interface
    precice::SolverInterface * precice_ = NULL;

    //- preCICE solver interface initialized
    bool preciceInitialized_ = false;

    //- Conjugate Heat Transfer module object
    CHT::ConjugateHeatTransfer * CHT_ = NULL;

    // Timesteps

        //- Timestep dictated by preCICE
        double timestepPrecice_;

        //- Timestep used by the solver
        double timestepSolver_;

        //- Stored (fixed) timestep
        double timestepStored_;

    // Checkpointing

        //- Checkpointed time (value)
        Foam::scalar couplingIterationTimeValue_;

        //- Checkpointed time (index)
        Foam::label couplingIterationTimeIndex_;

        // Vectors of pointers to the checkpointed fields and their copies

            //- Checkpointed volScalarField fields
            std::vector<Foam::volScalarField*> volScalarFields_;

            //- Checkpointed volScalarField fields (copies)
            std::vector<Foam::volScalarField*> volScalarFieldCopies_;

            //- Checkpointed volVectorField fields
            std::vector<Foam::volVectorField*> volVectorFields_;

            //- Checkpointed volVectorField fields (copies)
            std::vector<Foam::volVectorField*> volVectorFieldCopies_;

            //- Checkpointed surfaceScalarField fields
            std::vector<Foam::surfaceScalarField*> surfaceScalarFields_;

            //- Checkpointed surfaceScalarField fields (copies)
            std::vector<Foam::surfaceScalarField*> surfaceScalarFieldCopies_;

            //- Checkpointed surfaceVectorField fields
            std::vector<Foam::surfaceVectorField*> surfaceVectorFields_;

            //- Checkpointed surfaceVectorField fields (copies)
            std::vector<Foam::surfaceVectorField*> surfaceVectorFieldCopies_;

            // NOTE: Declare additional vectors for any other types required.

    // Configuration

        //- Read the adapter's configuration file
        bool configFileRead();

        //- Check the adapter's configuration file
        bool configFileCheck(const std::string adapterConfigFileName);

    // Methods communicating with preCICE

        //- Initialize preCICE and exchange the first data
        void initialize();

        //- Finalize and destroy preCICE
        void finalize();

        //- Advance preCICE
        void advance();

        //- Read the coupling data at each interface
        void readCouplingData();

        //- Write the coupling data at each interface
        void writeCouplingData();

        //- Adjust the timestep of the solver according to preCICE
        void adjustSolverTimeStep();

        //- Determine if the coupling is still happening
        bool isCouplingOngoing();

        //- Determine if the coupling timestep has been completed
        bool isCouplingTimestepComplete();

        //- Determine if a checkpoint must be read
        bool isReadCheckpointRequired();

        //- Determine if a checkpoint must be written
        bool isWriteCheckpointRequired();

        //- Tell preCICE that the checkpoint has been read
        void fulfilledReadCheckpoint();

        //- Tell preCICE that the checkpoint has been written
        void fulfilledWriteCheckpoint();

    // Methods for checkpointing

        //- Configure the checkpointing
        void setupCheckpointing();

        //- Make a copy of the runTime object
        void storeCheckpointTime();

        //- Restore the copy of the runTime object
        void reloadCheckpointTime();

        // Add checkpoint fields, depending on the type

            //- Add a volScalarField to checkpoint
            void addCheckpointField(volScalarField & field);

            //- Add a volVectorField to checkpoint
            void addCheckpointField(volVectorField & field);

            //- Add a surfaceScalarField to checkpoint
            void addCheckpointField(surfaceScalarField & field);

            //- Add a surfaceVectorField to checkpoint
            void addCheckpointField(surfaceVectorField & field);

            // NOTE: Add here methods to add other object types to checkpoint,
            // if needed.

        //- Read the checkpoint - restore the fields and time
        void readCheckpoint();

        //- Write the checkpoint - store the fields and time
        void writeCheckpoint();

    //- Destroy the preCICE interface and delete the allocated
    //  memory in a proper way. Called by the destructor.
    void teardown();

public:

    // Methods called by the functionObject

        //- Constructor
        Adapter(const Foam::Time& runTime, const Foam::fvMesh& mesh);

        //- Setup the adapter's configuration
        void configure();

        //- Called by the functionObject's execute()
        void execute();

        //- Called by the functionObject's adjustTimeStep()
        void adjustTimeStep();

        //- Called by the functionObject's end()
        void end();

        //- Destructor
        ~Adapter();

};

}

#endif
