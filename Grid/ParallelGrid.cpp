#include "ParallelGrid.h"

#if defined (PARALLEL_GRID)

#ifdef PRINT_MESSAGE

// Names of buffers of parallel grid for debug purposes.
const char* BufferPositionNames[] =
{
#define FUNCTION(X) #X,
#include "BufferPosition.inc.h"
#undef FUNCTION
};

#endif /* PRINT_MESSAGE */

ParallelGrid::ParallelGrid (const ParallelGridCoordinate& totSize,
              const ParallelGridCoordinate& bufSizeL, const ParallelGridCoordinate& bufSizeR,
              const int process, const int totalProc, uint32_t step) :
  ParallelGridBase (step),
  totalSize (totSize),
  bufferSizeLeft (bufSizeL),
  bufferSizeRight (bufSizeR),
  processId (process),
  totalProcCount (totalProc)
{
  ASSERT (bufferSizeLeft == bufferSizeRight);

#if defined (ONE_TIME_STEP)
  grid_iter numTimeStepsInBuild = 2;
#endif
#if defined (TWO_TIME_STEPS)
  grid_iter numTimeStepsInBuild = 3;
#endif

  oppositeDirections.resize (BUFFER_COUNT);
  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    oppositeDirections[i] = getOpposite ((BufferPosition) i);
  }

  sendStart.resize (BUFFER_COUNT);
  sendEnd.resize (BUFFER_COUNT);
  recvStart.resize (BUFFER_COUNT);
  recvEnd.resize (BUFFER_COUNT);

  directions.resize (BUFFER_COUNT);

  buffersSend.resize (BUFFER_COUNT);
  buffersReceive.resize (BUFFER_COUNT);

  // Call specific constructor.
  ParallelGridConstructor (numTimeStepsInBuild);

  doShare.resize (BUFFER_COUNT);
  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    getShare ((BufferPosition) i, doShare[i]);
  }

  SendReceiveCoordinatesInit ();

  gridValues.resize (size.calculateTotalCoord ());

#if PRINT_MESSAGE
  printf ("New grid for proc: %d (of %d) with raw size: %lu.\n", process, totalProcCount, gridValues.size ());
#endif
}

void
ParallelGrid::SendReceiveCoordinatesInit ()
{
#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  SendReceiveCoordinatesInit1D_X ();
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  SendReceiveCoordinatesInit1D_Y ();
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  SendReceiveCoordinatesInit1D_Z ();
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  SendReceiveCoordinatesInit2D_XY ();
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  SendReceiveCoordinatesInit2D_YZ ();
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  SendReceiveCoordinatesInit2D_XZ ();
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  SendReceiveCoordinatesInit3D_XYZ ();
#endif
}


#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
void
ParallelGrid::SendReceiveCoordinatesInit1D_X ()
{
  sendStart[LEFT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  sendEnd[LEFT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  recvStart[LEFT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  recvEnd[LEFT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  sendStart[RIGHT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  sendEnd[RIGHT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  recvStart[RIGHT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    0
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  recvEnd[RIGHT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );
}
#endif


#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
void
ParallelGrid::SendReceiveCoordinatesInit1D_Y ()
{
  sendStart[DOWN] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  sendEnd[DOWN] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , 2 * bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  recvStart[DOWN] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  recvEnd[DOWN] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  sendStart[UP] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - 2 * bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  sendEnd[UP] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  recvStart[UP] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , 0
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  recvEnd[UP] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );
}
#endif


#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
void
ParallelGrid::SendReceiveCoordinatesInit1D_Z ()
{
  sendStart[BACK] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  sendEnd[BACK] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , 2 * bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );

  recvStart[BACK] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  recvEnd[BACK] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ ()
#endif
#endif
#endif
  );

  sendStart[FRONT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  sendEnd[FRONT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
#endif
  );

  recvStart[FRONT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , 0
#endif
#endif
#endif
  );

  recvEnd[FRONT] = ParallelGridCoordinate (
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
#if defined (GRID_2D) || defined (GRID_3D)
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
#endif
  );
}
#endif


#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
void
ParallelGrid::SendReceiveCoordinatesInit2D_XY ()
{
  sendStart[LEFT_DOWN] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendEnd[LEFT_DOWN] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
    , 2 * bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[LEFT_DOWN] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  recvEnd[LEFT_DOWN] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
    , size.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  sendStart[LEFT_UP] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - 2 * bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendEnd[LEFT_UP] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[LEFT_UP] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , 0
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  recvEnd[LEFT_UP] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  sendStart[RIGHT_DOWN] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendEnd[RIGHT_DOWN] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , 2 * bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[RIGHT_DOWN] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    0
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  recvEnd[RIGHT_DOWN] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  sendStart[RIGHT_UP] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
    , size.getY () - 2 * bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendEnd[RIGHT_UP] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[RIGHT_UP] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    0
    , 0
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  recvEnd[RIGHT_UP] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );
}
#endif


#if defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
void
ParallelGrid::SendReceiveCoordinatesInit2D_YZ ()
{
  sendStart[DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendEnd[DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , 2 * bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , 2 * bufferSizeLeft.getZ ()
#endif
#endif
  );

  recvStart[DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvEnd[DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY ()
#if defined (GRID_3D)
    , size.getZ ()
#endif
#endif
  );

  sendStart[DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
#endif
  );

  sendEnd[DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , 2 * bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , 0
#endif
#endif
  );

  recvEnd[DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendStart[UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - 2 * bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendEnd[UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , 2 * bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , 0
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvEnd[UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ ()
#endif
#endif
  );

  sendStart[UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - 2 * bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
#endif
  );

  sendEnd[UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , 0
#if defined (GRID_3D)
    , 0
#endif
#endif
  );

  recvEnd[UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );
}
#endif


#if defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
void
ParallelGrid::SendReceiveCoordinatesInit2D_XZ ()
{
  sendStart[LEFT_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendEnd[LEFT_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , 2 * bufferSizeLeft.getZ ()
#endif
#endif
  );

  recvStart[LEFT_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvEnd[LEFT_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ ()
#endif
#endif
  );

  sendStart[LEFT_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
#endif
  );

  sendEnd[LEFT_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[LEFT_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , 0
#endif
#endif
  );

  recvEnd[LEFT_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendStart[RIGHT_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );

  sendEnd[RIGHT_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , 2 * bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[RIGHT_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    0
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvEnd[RIGHT_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ ()
#endif
#endif
  );

  sendStart[RIGHT_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
#endif
  );

  sendEnd[RIGHT_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , size.getZ () - bufferSizeRight.getZ ()
#endif
#endif
  );

  recvStart[RIGHT_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    0
    , bufferSizeLeft.getY ()
#if defined (GRID_3D)
    , 0
#endif
#endif
  );

  recvEnd[RIGHT_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
#if defined (GRID_3D)
    , bufferSizeLeft.getZ ()
#endif
#endif
  );
}
#endif


#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
void
ParallelGrid::SendReceiveCoordinatesInit3D_XYZ ()
{
  sendStart[LEFT_DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeLeft.getY ()
    , bufferSizeLeft.getZ ()
#endif
  );

  sendEnd[LEFT_DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
    , 2 * bufferSizeLeft.getY ()
    , 2 * bufferSizeLeft.getZ ()
#endif
  );

  recvStart[LEFT_DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
    , size.getZ () - bufferSizeRight.getZ ()
#endif
  );

  recvEnd[LEFT_DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
    , size.getY ()
    , size.getZ ()
#endif
  );

  sendStart[LEFT_DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeLeft.getY ()
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
  );

  sendEnd[LEFT_DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
    , 2 * bufferSizeLeft.getY ()
    , size.getZ () - bufferSizeRight.getZ ()
#endif
  );

  recvStart[LEFT_DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
    , 0
#endif
  );

  recvEnd[LEFT_DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
    , size.getY ()
    , bufferSizeLeft.getZ ()
#endif
  );

  sendStart[LEFT_UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - 2 * bufferSizeRight.getY ()
    , bufferSizeLeft.getZ ()
#endif
  );

  sendEnd[LEFT_UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
    , 2 * bufferSizeLeft.getZ ()
#endif
  );

  recvStart[LEFT_UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , 0
    , size.getZ () - bufferSizeRight.getZ ()
#endif
  );

  recvEnd[LEFT_UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
    , bufferSizeLeft.getY ()
    , size.getZ ()
#endif
  );

  sendStart[LEFT_UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY () - 2 * bufferSizeRight.getY ()
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
  );

  sendEnd[LEFT_UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_3D)
    2 * bufferSizeLeft.getX ()
    , size.getY () - bufferSizeRight.getY ()
    , size.getZ () - bufferSizeRight.getZ ()
#endif
  );

  recvStart[LEFT_UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , 0
    , 0
#endif
  );

  recvEnd[LEFT_UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    size.getX ()
    , bufferSizeLeft.getY ()
    , bufferSizeLeft.getZ ()
#endif
  );


  sendStart[RIGHT_DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
    , bufferSizeLeft.getZ ()
#endif
  );

  sendEnd[RIGHT_DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , 2 * bufferSizeLeft.getY ()
    , 2 * bufferSizeLeft.getZ ()
#endif
  );

  recvStart[RIGHT_DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    0
    , size.getY () - bufferSizeRight.getY ()
    , size.getZ () - bufferSizeRight.getZ ()
#endif
  );

  recvEnd[RIGHT_DOWN_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY ()
    , size.getZ ()
#endif
  );

  sendStart[RIGHT_DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
    , bufferSizeLeft.getY ()
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
  );

  sendEnd[RIGHT_DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , 2 * bufferSizeLeft.getY ()
    , size.getZ () - bufferSizeRight.getZ ()
#endif
  );

  recvStart[RIGHT_DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    0
    , size.getY () - bufferSizeRight.getY ()
    , 0
#endif
  );

  recvEnd[RIGHT_DOWN_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , size.getY ()
    , bufferSizeLeft.getZ ()
#endif
  );

  sendStart[RIGHT_UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
    , size.getY () - 2 * bufferSizeRight.getY ()
    , bufferSizeLeft.getZ ()
#endif
  );

  sendEnd[RIGHT_UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
    , 2 * bufferSizeLeft.getZ ()
#endif
  );

  recvStart[RIGHT_UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    0
    , 0
    , size.getZ () - bufferSizeRight.getZ ()
#endif
  );

  recvEnd[RIGHT_UP_BACK] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeLeft.getY ()
    , size.getZ ()
#endif
  );

  sendStart[RIGHT_UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_3D)
    size.getX () - 2 * bufferSizeRight.getX ()
    , size.getY () - 2 * bufferSizeRight.getY ()
    , size.getZ () - 2 * bufferSizeRight.getZ ()
#endif
  );

  sendEnd[RIGHT_UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_3D)
    size.getX () - bufferSizeRight.getX ()
    , size.getY () - bufferSizeRight.getY ()
    , size.getZ () - bufferSizeRight.getZ ()
#endif
  );

  recvStart[RIGHT_UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    0
    , 0
    , 0
#endif
  );

  recvEnd[RIGHT_UP_FRONT] = ParallelGridCoordinate (
#if defined (GRID_2D) || defined (GRID_3D)
    bufferSizeLeft.getX ()
    , bufferSizeLeft.getY ()
    , bufferSizeLeft.getZ ()
#endif
  );
}
#endif



void
ParallelGrid::SendRawBuffer (BufferPosition buffer, int processTo)
{
#if PRINT_MESSAGE
  printf ("\t\tSend RAW. PID=#%d. Direction TO=%s, size=%lu.\n",
    processId, BufferPositionNames[buffer], buffersReceive[buffer].size ());
#endif
  MPI_Status status;

  FieldValue* rawBuffer = buffersSend[buffer].data ();

#ifdef FLOAT_VALUES
  int retCode = MPI_Send (rawBuffer, buffersSend[buffer].size (), MPI_FLOAT,
                          processTo, processId, MPI_COMM_WORLD);
#endif /* FLOAT_VALUES */
#ifdef DOUBLE_VALUES
  int retCode = MPI_Send (rawBuffer, buffersSend[buffer].size (), MPI_DOUBLE,
                          processTo, processId, MPI_COMM_WORLD);
#endif /* DOUBLE_VALUES */
#ifdef LONG_DOUBLE_VALUES
  int retCode = MPI_Send (rawBuffer, buffersSend[buffer].size (), MPI_LONG_DOUBLE,
                          processTo, processId, MPI_COMM_WORLD);
#endif /* LONG_DOUBLE_VALUES */

  ASSERT (retCode == MPI_SUCCESS);
}

void
ParallelGrid::ReceiveRawBuffer (BufferPosition buffer, int processFrom)
{
#if PRINT_MESSAGE
  printf ("\t\tReceive RAW. PID=#%d. Direction FROM=%s, size=%lu.\n",
    processId, BufferPositionNames[buffer], buffersReceive[buffer].size ());
#endif
  MPI_Status status;

  FieldValue* rawBuffer = buffersReceive[buffer].data ();

#ifdef FLOAT_VALUES
  int retCode = MPI_Recv (rawBuffer, buffersReceive[buffer].size (), MPI_FLOAT,
                          processFrom, processFrom, MPI_COMM_WORLD, &status);
#endif /* FLOAT_VALUES */
#ifdef DOUBLE_VALUES
  int retCode = MPI_Recv (rawBuffer, buffersReceive[buffer].size (), MPI_DOUBLE,
                          processFrom, processFrom, MPI_COMM_WORLD, &status);
#endif /* DOUBLE_VALUES */
#ifdef LONG_DOUBLE_VALUES
  int retCode = MPI_Recv (rawBuffer, buffersReceive[buffer].size (), MPI_LONG_DOUBLE,
                          processFrom, processFrom, MPI_COMM_WORLD, &status);
#endif /* LONG_DOUBLE_VALUES */

  ASSERT (retCode == MPI_SUCCESS);
}

void
ParallelGrid::SendReceiveRawBuffer (BufferPosition bufferSend, int processTo,
                            BufferPosition bufferReceive, int processFrom)
{
#if PRINT_MESSAGE
  printf ("\t\tSend/Receive RAW. PID=#%d. Directions TO=%s FROM=%s. Size TO=%lu FROM=%lu.\n",
    processId, BufferPositionNames[bufferSend], BufferPositionNames[bufferReceive],
    buffersReceive[bufferSend].size (), buffersReceive[bufferReceive].size ());
#endif
  MPI_Status status;

  FieldValue* rawBufferSend = buffersSend[bufferSend].data ();
  FieldValue* rawBufferReceive = buffersReceive[bufferReceive].data ();

#ifdef FLOAT_VALUES
  int retCode = MPI_Sendrecv (rawBufferSend, buffersSend[bufferSend].size (), MPI_FLOAT,
                              processTo, processId,
                              rawBufferReceive, buffersReceive[bufferReceive].size (), MPI_FLOAT,
                              processFrom, processFrom,
                              MPI_COMM_WORLD, &status);
#endif /* FLOAT_VALUES */
#ifdef DOUBLE_VALUES
  int retCode = MPI_Sendrecv (rawBufferSend, buffersSend[bufferSend].size (), MPI_DOUBLE,
                              processTo, processId,
                              rawBufferReceive, buffersReceive[bufferReceive].size (), MPI_DOUBLE,
                              processFrom, processFrom,
                              MPI_COMM_WORLD, &status);
#endif /* DOUBLE_VALUES */
#ifdef LONG_DOUBLE_VALUES
  int retCode = MPI_Sendrecv (rawBufferSend, buffersSend[bufferSend].size (), MPI_LONG_DOUBLE,
                              processTo, processId,
                              rawBufferReceive, buffersReceive[bufferReceive].size (), MPI_LONG_DOUBLE,
                              processFrom, processFrom,
                              MPI_COMM_WORLD, &status);
#endif /* LONG_DOUBLE_VALUES */


  ASSERT (retCode == MPI_SUCCESS);
}

/**
 * Send/receive method to be called for all grid types.
 */
void
ParallelGrid::SendReceive ()
{
// #if PRINT_MESSAGE
//   printf ("Send/Receive %d\n", processId);
// #endif

  // Go through all directions and send/receive.
  for (int buf = 0; buf < BUFFER_COUNT; ++buf)
  {
    SendReceiveBuffer ((BufferPosition) buf);
  }
}

void
ParallelGrid::Share ()
{
  SendReceive ();

  MPI_Barrier (MPI_COMM_WORLD);
}

#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || defined (PARALLEL_BUFFER_DIMENSION_1D_Z)
void
ParallelGrid::CalculateGridSizeForNode (grid_coord& c1, int nodeGridSize1, bool has1, grid_coord size1)
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || defined (PARALLEL_BUFFER_DIMENSION_2D_XZ)
void
ParallelGrid::CalculateGridSizeForNode (grid_coord& c1, int nodeGridSize1, bool has1, grid_coord size1,
                                grid_coord& c2, int nodeGridSize2, bool has2, grid_coord size2)
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
void
ParallelGrid::CalculateGridSizeForNode (grid_coord& c1, int nodeGridSize1, bool has1, grid_coord size1,
                                grid_coord& c2, int nodeGridSize2, bool has2, grid_coord size2,
                                grid_coord& c3, int nodeGridSize3, bool has3, grid_coord size3)
#endif
{
#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || defined (PARALLEL_BUFFER_DIMENSION_1D_Z) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (has1)
    c1 = size1 / nodeGridSize1;
  else
    c1 = size1 - (nodeGridSize1 - 1) * (size1 / nodeGridSize1);
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (has2)
    c2 = size2 / nodeGridSize2;
  else
    c2 = size2 - (nodeGridSize2 - 1) * (size2 / nodeGridSize2);
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (has3)
    c3 = size3 / nodeGridSize3;
  else
    c3 = size3 - (nodeGridSize3 - 1) * (size3 / nodeGridSize3);
#endif
}

BufferPosition
ParallelGrid::getOpposite (BufferPosition direction)
{
  switch (direction)
  {
#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case LEFT:
      return RIGHT;
    case RIGHT:
      return LEFT;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case UP:
      return DOWN;
    case DOWN:
      return UP;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case FRONT:
      return BACK;
    case BACK:
      return FRONT;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case LEFT_UP:
      return RIGHT_DOWN;
    case LEFT_DOWN:
      return RIGHT_UP;
    case RIGHT_UP:
      return LEFT_DOWN;
    case RIGHT_DOWN:
      return LEFT_UP;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case LEFT_FRONT:
      return RIGHT_BACK;
    case LEFT_BACK:
      return RIGHT_FRONT;
    case RIGHT_FRONT:
      return LEFT_BACK;
    case RIGHT_BACK:
      return LEFT_FRONT;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case UP_FRONT:
      return DOWN_BACK;
    case UP_BACK:
      return DOWN_FRONT;
    case DOWN_FRONT:
      return UP_BACK;
    case DOWN_BACK:
      return UP_FRONT;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case LEFT_UP_FRONT:
      return RIGHT_DOWN_BACK;
    case LEFT_UP_BACK:
      return RIGHT_DOWN_FRONT;
    case LEFT_DOWN_FRONT:
      return RIGHT_UP_BACK;
    case LEFT_DOWN_BACK:
      return RIGHT_UP_FRONT;
    case RIGHT_UP_FRONT:
      return LEFT_DOWN_BACK;
    case RIGHT_UP_BACK:
      return LEFT_DOWN_FRONT;
    case RIGHT_DOWN_FRONT:
      return LEFT_UP_BACK;
    case RIGHT_DOWN_BACK:
      return LEFT_UP_FRONT;
#endif
    default:
    {
      UNREACHABLE;
    }
  }
}

void
ParallelGrid::getShare (BufferPosition direction, std::pair<bool, bool>& pair)
{
  bool doSend = true;
  bool doReceive = true;

  switch (direction)
  {
#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case LEFT:
    {
      if (!hasL)
      {
        doSend = false;
      }
      else if (!hasR)
      {
        doReceive = false;
      }

      break;
    }
    case RIGHT:
    {
      if (!hasL)
      {
        doReceive = false;
      }
      else if (!hasR)
      {
        doSend = false;
      }

      break;
    }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case UP:
    {
      if (!hasD)
      {
        doReceive = false;
      }
      if (!hasU)
      {
        doSend = false;
      }

      break;
    }
    case DOWN:
    {
      if (!hasD)
      {
        doSend = false;
      }
      else if (!hasU)
      {
        doReceive = false;
      }

      break;
    }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case FRONT:
    {
      if (!hasB)
      {
        doReceive = false;
      }
      else if (!hasF)
      {
        doSend = false;
      }

      break;
    }
    case BACK:
    {
      if (!hasB)
      {
        doSend = false;
      }
      else if (!hasF)
      {
        doReceive = false;
      }

      break;
    }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case LEFT_UP:
    {
      if (!hasR || !hasD)
      {
        doReceive = false;
      }
      if (!hasL || !hasU)
      {
        doSend = false;
      }

      break;
    }
    case LEFT_DOWN:
    {
      if (!hasL || !hasD)
      {
        doSend = false;
      }
      if (!hasR || !hasU)
      {
        doReceive = false;
      }

      break;
    }
    case RIGHT_UP:
    {
      if (!hasL || !hasD)
      {
        doReceive = false;
      }
      if (!hasR || !hasU)
      {
        doSend = false;
      }

      break;
    }
    case RIGHT_DOWN:
    {
      if (!hasR || !hasD)
      {
        doSend = false;
      }
      if (!hasL || !hasU)
      {
        doReceive = false;
      }

      break;
    }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case LEFT_FRONT:
    {
      if (!hasR || !hasB)
      {
        doReceive = false;
      }
      if (!hasL || !hasF)
      {
        doSend = false;
      }

      break;
    }
    case LEFT_BACK:
    {
      if (!hasL || !hasB)
      {
        doSend = false;
      }
      if (!hasR || !hasF)
      {
        doReceive = false;
      }

      break;
    }
    case RIGHT_FRONT:
    {
      if (!hasL || !hasB)
      {
        doReceive = false;
      }
      if (!hasR || !hasF)
      {
        doSend = false;
      }

      break;
    }
    case RIGHT_BACK:
    {
      if (!hasR || !hasB)
      {
        doSend = false;
      }
      if (!hasL || !hasF)
      {
        doReceive = false;
      }

      break;
    }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case UP_FRONT:
    {
      if (!hasD || !hasB)
      {
        doReceive = false;
      }
      if (!hasU || !hasF)
      {
        doSend = false;
      }

      break;
    }
    case UP_BACK:
    {
      if (!hasU || !hasB)
      {
        doSend = false;
      }
      if (!hasD || !hasF)
      {
        doReceive = false;
      }

      break;
    }
    case DOWN_FRONT:
    {
      if (!hasU || !hasB)
      {
        doReceive = false;
      }
      if (!hasD || !hasF)
      {
        doSend = false;
      }

      break;
    }
    case DOWN_BACK:
    {
      if (!hasD || !hasB)
      {
        doSend = false;
      }
      if (!hasU || !hasF)
      {
        doReceive = false;
      }

      break;
    }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
    case LEFT_UP_FRONT:
    {
      if (!hasR || !hasD || !hasB)
      {
        doReceive = false;
      }
      if (!hasL || !hasU || !hasF)
      {
        doSend = false;
      }

      break;
    }
    case LEFT_UP_BACK:
    {
      if (!hasR || !hasD || !hasF)
      {
        doReceive = false;
      }
      if (!hasL || !hasU || !hasB)
      {
        doSend = false;
      }

      break;
    }
    case LEFT_DOWN_FRONT:
    {
      if (!hasR || !hasU || !hasB)
      {
        doReceive = false;
      }
      if (!hasL || !hasD || !hasF)
      {
        doSend = false;
      }

      break;
    }
    case LEFT_DOWN_BACK:
    {
      if (!hasR || !hasU || !hasF)
      {
        doReceive = false;
      }
      if (!hasL || !hasD || !hasB)
      {
        doSend = false;
      }

      break;
    }
    case RIGHT_UP_FRONT:
    {
      if (!hasL || !hasD || !hasB)
      {
        doReceive = false;
      }
      if (!hasR || !hasU || !hasF)
      {
        doSend = false;
      }

      break;
    }
    case RIGHT_UP_BACK:
    {
      if (!hasL || !hasD || !hasF)
      {
        doReceive = false;
      }
      if (!hasR || !hasU || !hasB)
      {
        doSend = false;
      }

      break;
    }
    case RIGHT_DOWN_FRONT:
    {
      if (!hasL || !hasU || !hasB)
      {
        doReceive = false;
      }
      if (!hasR || !hasD || !hasF)
      {
        doSend = false;
      }

      break;
    }
    case RIGHT_DOWN_BACK:
    {
      if (!hasL || !hasU || !hasF)
      {
        doReceive = false;
      }
      if (!hasR || !hasD || !hasB)
      {
        doSend = false;
      }

      break;
    }
#endif
    default:
    {
      UNREACHABLE;
    }
  }

  pair.first = doSend;
  pair.second = doReceive;
}

void
ParallelGrid::InitDirections ()
{
#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  directions[LEFT] = processId - 1;
  directions[RIGHT] = processId + 1;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ)
  directions[DOWN] = processId - 1;
  directions[UP] = processId + 1;
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  directions[DOWN] = processId - nodeGridSizeX;
  directions[UP] = processId + nodeGridSizeX;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z)
  directions[BACK] = processId - 1;
  directions[FRONT] = processId + 1;
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_2D_YZ)
  directions[BACK] = processId - nodeGridSizeY;
  directions[FRONT] = processId + nodeGridSizeY;
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_2D_XZ)
  directions[BACK] = processId - nodeGridSizeX;
  directions[FRONT] = processId + nodeGridSizeX;
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  directions[BACK] = processId - nodeGridSizeXY;
  directions[FRONT] = processId + nodeGridSizeXY;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  directions[LEFT_DOWN] = processId - nodeGridSizeX - 1;
  directions[LEFT_UP] = processId + nodeGridSizeX - 1;
  directions[RIGHT_DOWN] = processId - nodeGridSizeX + 1;
  directions[RIGHT_UP] = processId + nodeGridSizeX + 1;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_YZ)
  directions[DOWN_BACK] = processId - nodeGridSizeY - 1;
  directions[DOWN_FRONT] = processId + nodeGridSizeY - 1;
  directions[UP_BACK] = processId - nodeGridSizeY + 1;
  directions[UP_FRONT] = processId + nodeGridSizeY + 1;
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  directions[DOWN_BACK] = processId - nodeGridSizeXY - nodeGridSizeX;
  directions[DOWN_FRONT] = processId + nodeGridSizeXY - nodeGridSizeX;
  directions[UP_BACK] = processId - nodeGridSizeXY + nodeGridSizeX;
  directions[UP_FRONT] = processId + nodeGridSizeXY + nodeGridSizeX;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XZ)
  directions[LEFT_BACK] = processId - nodeGridSizeX - 1;
  directions[LEFT_FRONT] = processId + nodeGridSizeX - 1;
  directions[RIGHT_BACK] = processId - nodeGridSizeX + 1;
  directions[RIGHT_FRONT] = processId + nodeGridSizeX + 1;
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  directions[LEFT_BACK] = processId - nodeGridSizeXY - 1;
  directions[LEFT_FRONT] = processId + nodeGridSizeXY - 1;
  directions[RIGHT_BACK] = processId - nodeGridSizeXY + 1;
  directions[RIGHT_FRONT] = processId + nodeGridSizeXY + 1;
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  directions[LEFT_DOWN_BACK] = processId - nodeGridSizeXY - nodeGridSizeX - 1;
  directions[LEFT_DOWN_FRONT] = processId + nodeGridSizeXY - nodeGridSizeX - 1;
  directions[LEFT_UP_BACK] = processId - nodeGridSizeXY + nodeGridSizeX - 1;
  directions[LEFT_UP_FRONT] = processId + nodeGridSizeXY + nodeGridSizeX - 1;
  directions[RIGHT_DOWN_BACK] = processId - nodeGridSizeXY - nodeGridSizeX + 1;
  directions[RIGHT_DOWN_FRONT] = processId + nodeGridSizeXY - nodeGridSizeX + 1;
  directions[RIGHT_UP_BACK] = processId - nodeGridSizeXY + nodeGridSizeX + 1;
  directions[RIGHT_UP_FRONT] = processId + nodeGridSizeXY + nodeGridSizeX + 1;
#endif
}

void
ParallelGrid::InitBufferFlags ()
{
#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  hasL = false;
  hasR = false;

#if defined (PARALLEL_BUFFER_DIMENSION_1D_X)
  if (processId > 0)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
      defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (processId % nodeGridSizeX > 0)
#endif
  {
    hasL = true;
  }

#if defined (PARALLEL_BUFFER_DIMENSION_1D_X)
  if (processId < nodeGridSizeX - 1)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || \
      defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (processId % nodeGridSizeX < nodeGridSizeX - 1)
#endif
  {
    hasR = true;
  }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  hasU = false;
  hasD = false;

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y)
  if (processId > 0)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_XY)
  if (processId >= nodeGridSizeX)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_YZ)
  if (processId % nodeGridSizeY > 0)
#elif defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if ((processId % (nodeGridSizeXY)) >= nodeGridSizeX)
#endif
  {
    hasD = true;
  }

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y)
  if (processId < nodeGridSizeY - 1)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_XY)
  if (processId < nodeGridSizeXY - nodeGridSizeX)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_YZ)
  if (processId % nodeGridSizeY < nodeGridSizeY - 1)
#elif defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if ((processId % (nodeGridSizeXY)) < nodeGridSizeXY - nodeGridSizeX)
#endif
  {
    hasU = true;
  }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z) || defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  hasF = false;
  hasB = false;

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z)
  if (processId > 0)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_YZ)
  if (processId >= nodeGridSizeY)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_XZ)
  if (processId >= nodeGridSizeX)
#elif defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (processId >= nodeGridSizeXY)
#endif
  {
    hasB = true;
  }

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z)
  if (processId < nodeGridSizeZ - 1)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_YZ)
  if (processId < nodeGridSizeYZ - nodeGridSizeY)
#elif defined (PARALLEL_BUFFER_DIMENSION_2D_XZ)
  if (processId < nodeGridSizeXZ - nodeGridSizeX)
#elif defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (processId < nodeGridSizeXYZ - nodeGridSizeXY)
#endif
  {
    hasF = true;
  }
#endif
}

void
ParallelGrid::InitBuffers (grid_iter numTimeStepsInBuild)
{
#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (hasL)
  {
    int buf_size = bufferSizeLeft.getX () * numTimeStepsInBuild;
#if defined (GRID_2D) || defined (GRID_3D)
    buf_size *= currentSize.getY ();
#endif
#if defined (GRID_3D)
    buf_size *= currentSize.getZ ();
#endif
    buffersSend[LEFT].resize (buf_size);
    buffersReceive[LEFT].resize (buf_size);
  }
  if (hasR)
  {
    int buf_size = bufferSizeRight.getX () * numTimeStepsInBuild;
#if defined (GRID_2D) || defined (GRID_3D)
    buf_size *= currentSize.getY ();
#endif
#if defined (GRID_3D)
    buf_size *= currentSize.getZ ();
#endif
    buffersSend[RIGHT].resize (buf_size);
    buffersReceive[RIGHT].resize (buf_size);
  }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (hasD)
  {
    int buf_size = bufferSizeLeft.getY () * currentSize.getX () * numTimeStepsInBuild;
#if defined (GRID_3D)
    buf_size *= currentSize.getZ ();
#endif
    buffersSend[DOWN].resize (buf_size);
    buffersReceive[DOWN].resize (buf_size);
  }
  if (hasU)
  {
    int buf_size = bufferSizeRight.getY () * currentSize.getX () * numTimeStepsInBuild;
#if defined (GRID_3D)
    buf_size *= currentSize.getZ ();
#endif
    buffersSend[UP].resize (buf_size);
    buffersReceive[UP].resize (buf_size);
  }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z) || defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (hasB)
  {
    int buf_size = bufferSizeLeft.getZ () * currentSize.getY () * currentSize.getX () * numTimeStepsInBuild;
    buffersSend[BACK].resize (buf_size);
    buffersReceive[BACK].resize (buf_size);
  }
  if (hasF)
  {
    int buf_size = bufferSizeRight.getZ () * currentSize.getY () * currentSize.getX () * numTimeStepsInBuild;
    buffersSend[FRONT].resize (buf_size);
    buffersReceive[FRONT].resize (buf_size);
  }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (hasL && hasD)
  {
    int buf_size = bufferSizeLeft.getX () * bufferSizeLeft.getY () * numTimeStepsInBuild;
#if defined (GRID_3D)
    buf_size *= currentSize.getZ ();
#endif
    buffersSend[LEFT_DOWN].resize (buf_size);
    buffersReceive[LEFT_DOWN].resize (buf_size);
  }
  if (hasL && hasU)
  {
    int buf_size = bufferSizeLeft.getX () * bufferSizeRight.getY () * numTimeStepsInBuild;
#if defined (GRID_3D)
    buf_size *= currentSize.getZ ();
#endif
    buffersSend[LEFT_UP].resize (buf_size);
    buffersReceive[LEFT_UP].resize (buf_size);
  }
  if (hasR && hasD)
  {
    int buf_size = bufferSizeRight.getX () * bufferSizeLeft.getY () * numTimeStepsInBuild;
#if defined (GRID_3D)
    buf_size *= currentSize.getZ ();
#endif
    buffersSend[RIGHT_DOWN].resize (buf_size);
    buffersReceive[RIGHT_DOWN].resize (buf_size);
  }
  if (hasR && hasU)
  {
    int buf_size = bufferSizeRight.getX () * bufferSizeRight.getY () * numTimeStepsInBuild;
#if defined (GRID_3D)
    buf_size *= currentSize.getZ ();
#endif
    buffersSend[RIGHT_UP].resize (buf_size);
    buffersReceive[RIGHT_UP].resize (buf_size);
  }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (hasD && hasB)
  {
    int buf_size = bufferSizeLeft.getY () * bufferSizeLeft.getZ () * currentSize.getX () * numTimeStepsInBuild;
    buffersSend[DOWN_BACK].resize (buf_size);
    buffersReceive[DOWN_BACK].resize (buf_size);
  }
  if (hasD && hasF)
  {
    int buf_size = bufferSizeLeft.getY () * bufferSizeRight.getZ () * currentSize.getX () * numTimeStepsInBuild;
    buffersSend[DOWN_FRONT].resize (buf_size);
    buffersReceive[DOWN_FRONT].resize (buf_size);
  }
  if (hasU && hasB)
  {
    int buf_size = bufferSizeRight.getY () * bufferSizeLeft.getZ () * currentSize.getX () * numTimeStepsInBuild;
    buffersSend[UP_BACK].resize (buf_size);
    buffersReceive[UP_BACK].resize (buf_size);
  }
  if (hasU && hasF)
  {
    int buf_size = bufferSizeRight.getY () * bufferSizeRight.getZ () * currentSize.getX () * numTimeStepsInBuild;
    buffersSend[UP_FRONT].resize (buf_size);
    buffersReceive[UP_FRONT].resize (buf_size);
  }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (hasL && hasB)
  {
    int buf_size = bufferSizeLeft.getX () * bufferSizeLeft.getY () * currentSize.getY () * numTimeStepsInBuild;
    buffersSend[LEFT_BACK].resize (buf_size);
    buffersReceive[LEFT_BACK].resize (buf_size);
  }
  if (hasL && hasF)
  {
    int buf_size = bufferSizeLeft.getX () * bufferSizeRight.getY () * currentSize.getY () * numTimeStepsInBuild;
    buffersSend[LEFT_FRONT].resize (buf_size);
    buffersReceive[LEFT_FRONT].resize (buf_size);
  }
  if (hasR && hasB)
  {
    int buf_size = bufferSizeRight.getX () * bufferSizeLeft.getY () * currentSize.getY () * numTimeStepsInBuild;
    buffersSend[RIGHT_BACK].resize (buf_size);
    buffersReceive[RIGHT_BACK].resize (buf_size);
  }
  if (hasR && hasF)
  {
    int buf_size = bufferSizeRight.getX () * bufferSizeRight.getY () * currentSize.getY () * numTimeStepsInBuild;
    buffersSend[RIGHT_FRONT].resize (buf_size);
    buffersReceive[RIGHT_FRONT].resize (buf_size);
  }
#endif

#if defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  int buf_size = bufferSizeLeft.getX () * bufferSizeLeft.getY () * bufferSizeLeft.getZ () * numTimeStepsInBuild;
  if (hasL && hasD && hasB)
  {
    buffersSend[LEFT_DOWN_BACK].resize (buf_size);
    buffersReceive[LEFT_DOWN_BACK].resize (buf_size);
  }
  if (hasL && hasD && hasF)
  {
    buffersSend[LEFT_DOWN_FRONT].resize (buf_size);
    buffersReceive[LEFT_DOWN_FRONT].resize (buf_size);
  }
  if (hasL && hasU && hasB)
  {
    buffersSend[LEFT_UP_BACK].resize (buf_size);
    buffersReceive[LEFT_UP_BACK].resize (buf_size);
  }
  if (hasL && hasU && hasF)
  {
    buffersSend[LEFT_UP_FRONT].resize (buf_size);
    buffersReceive[LEFT_UP_FRONT].resize (buf_size);
  }

  if (hasR && hasD && hasB)
  {
    buffersSend[RIGHT_DOWN_BACK].resize (buf_size);
    buffersReceive[RIGHT_DOWN_BACK].resize (buf_size);
  }
  if (hasR && hasD && hasF)
  {
    buffersSend[RIGHT_DOWN_FRONT].resize (buf_size);
    buffersReceive[RIGHT_DOWN_FRONT].resize (buf_size);
  }
  if (hasR && hasU && hasB)
  {
    buffersSend[RIGHT_UP_BACK].resize (buf_size);
    buffersReceive[RIGHT_UP_BACK].resize (buf_size);
  }
  if (hasR && hasU && hasF)
  {
    buffersSend[RIGHT_UP_FRONT].resize (buf_size);
    buffersReceive[RIGHT_UP_FRONT].resize (buf_size);
  }
#endif
}

void
ParallelGrid::ParallelGridConstructor (grid_iter numTimeStepsInBuild)
{
  NodeGridInit ();

  // Return if node not used.
#ifdef PARALLEL_BUFFER_DIMENSION_3D_XYZ
  if (processId >= nodeGridSizeXYZ)
  {
    return;
  }
#endif

#ifdef PARALLEL_BUFFER_DIMENSION_2D_XY
  if (processId >= nodeGridSizeXY)
  {
    return;
  }
#endif

#ifdef PARALLEL_BUFFER_DIMENSION_2D_YZ
  if (processId >= nodeGridSizeYZ)
  {
    return;
  }
#endif

#ifdef PARALLEL_BUFFER_DIMENSION_2D_XZ
  if (processId >= nodeGridSizeXZ)
  {
    return;
  }
#endif

  InitBufferFlags ();
  currentSize = GridInit ();

  ParallelGridCoordinate bufferSizeLeftCurrent (bufferSizeLeft);
  ParallelGridCoordinate bufferSizeRightCurrent (bufferSizeRight);

#if defined (PARALLEL_BUFFER_DIMENSION_1D_X) || defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (!hasL)
  {
    bufferSizeLeftCurrent.setX (0);
  }
  if (!hasR)
  {
    bufferSizeRightCurrent.setX (0);
  }
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_1D_Y) || defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (!hasD)
  {
    bufferSizeLeftCurrent.setY (0);
  }
  if (!hasU)
  {
    bufferSizeRightCurrent.setY (0);
  }
#endif
#if defined (PARALLEL_BUFFER_DIMENSION_1D_Z) || defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || \
    defined (PARALLEL_BUFFER_DIMENSION_2D_XZ) || defined (PARALLEL_BUFFER_DIMENSION_3D_XYZ)
  if (!hasB)
  {
    bufferSizeLeftCurrent.setZ (0);
  }
  if (!hasF)
  {
    bufferSizeRightCurrent.setZ (0);
  }
#endif
  size = currentSize + bufferSizeLeftCurrent + bufferSizeRightCurrent;

  InitBuffers (numTimeStepsInBuild);
  InitDirections ();

#if PRINT_MESSAGE
#ifdef GRID_1D
  printf ("Grid size for #%d process: %d.\n", processId,
    currentSize.getX ());
#endif
#ifdef GRID_2D
  printf ("Grid size for #%d process: %dx%d.\n", processId,
    currentSize.getX (), currentSize.getY ());
#endif
#ifdef GRID_3D
  printf ("Grid size for #%d process: %dx%dx%d.\n", processId,
    currentSize.getX (), currentSize.getY (), currentSize.getZ ());
#endif
#endif
}

void
ParallelGrid::SendReceiveBuffer (BufferPosition bufferDirection)
{
  // Return if node not used.
#ifdef PARALLEL_BUFFER_DIMENSION_3D_XYZ
  if (processId >= nodeGridSizeXYZ)
  {
    return;
  }
#endif

#ifdef PARALLEL_BUFFER_DIMENSION_2D_XY
  if (processId >= nodeGridSizeXY)
  {
    return;
  }
#endif

#ifdef PARALLEL_BUFFER_DIMENSION_2D_YZ
  if (processId >= nodeGridSizeYZ)
  {
    return;
  }
#endif

#ifdef PARALLEL_BUFFER_DIMENSION_2D_XZ
  if (processId >= nodeGridSizeXZ)
  {
    return;
  }
#endif

  bool doSend = doShare[bufferDirection].first;
  bool doReceive = doShare[bufferDirection].second;

  // Copy to send buffer
  if (doSend)
  {
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    for (grid_iter index = 0, i = sendStart[bufferDirection].getX ();
         i < sendEnd[bufferDirection].getX (); ++i)
    {
#endif
#if defined (GRID_2D) || defined (GRID_3D)
      for (grid_coord j = sendStart[bufferDirection].getY ();
           j < sendEnd[bufferDirection].getY (); ++j)
      {
#endif
#if defined (GRID_3D)
        for (grid_coord k = sendStart[bufferDirection].getZ ();
             k < sendEnd[bufferDirection].getZ (); ++k)
        {
#endif
#if defined (GRID_1D)
          ParallelGridCoordinate pos (i);
#endif
#if defined (GRID_2D)
          ParallelGridCoordinate pos (i, j);
#endif
#if defined (GRID_3D)
          ParallelGridCoordinate pos (i, j, k);
#endif

          FieldPointValue* val = getFieldPointValue (pos);
          buffersSend[bufferDirection][index++] = val->getCurValue ();
#if defined (ONE_TIME_STEP) || defined (TWO_TIME_STEPS)
          buffersSend[bufferDirection][index++] = val->getPrevValue ();
#if defined (TWO_TIME_STEPS)
          buffersSend[bufferDirection][index++] = val->getPrevPrevValue ();
#endif /* TWO_TIME_STEPS */
#endif /* ONE_TIME_STEP || TWO_TIME_STEPS */

#if defined (GRID_3D)
        }
#endif
#if defined (GRID_2D) || defined (GRID_3D)
      }
#endif
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    }
#endif
  }

  BufferPosition opposite = oppositeDirections[bufferDirection];
  int processTo = directions[bufferDirection];
  int processFrom = directions[opposite];

#if PRINT_MESSAGE
  printf ("\tSHARE RAW. PID=#%d. Directions TO(%d)=%s=#%d, FROM(%d)=%s=#%d.\n",
    processId, doSend, BufferPositionNames[bufferDirection], processTo,
               doReceive, BufferPositionNames[opposite], processFrom);
#endif

  if (doSend && !doReceive)
  {
    SendRawBuffer (bufferDirection, processTo);
  }
  else if (!doSend && doReceive)
  {
    ReceiveRawBuffer (opposite, processFrom);
  }
  else if (doSend && doReceive)
  {
    SendReceiveRawBuffer (bufferDirection, processTo, opposite, processFrom);
  }
  else
  {
    // Do nothing
  }

  // Copy from receive buffer
  if (doReceive)
  {
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    for (grid_iter index = 0, i = recvStart[bufferDirection].getX ();
         i < recvEnd[bufferDirection].getX (); ++i)
    {
#endif
#if defined (GRID_2D) || defined (GRID_3D)
      for (grid_coord j = recvStart[bufferDirection].getY ();
           j < recvEnd[bufferDirection].getY (); ++j)
      {
#endif
#if defined (GRID_3D)
        for (grid_coord k = recvStart[bufferDirection].getZ ();
             k < recvEnd[bufferDirection].getZ (); ++k)
        {
#endif

#if defined (TWO_TIME_STEPS)
          FieldPointValue* val = new FieldPointValue (buffersReceive[opposite][index++],
                                                      buffersReceive[opposite][index++],
                                                      buffersReceive[opposite][index++]);
#else /* TWO_TIME_STEPS */
#if defined (ONE_TIME_STEP)
          FieldPointValue* val = new FieldPointValue (buffersReceive[opposite][index++],
                                                      buffersReceive[opposite][index++]);
#else /* ONE_TIME_STEP */
          FieldPointValue* val = new FieldPointValue (buffersReceive[opposite][index++]);
#endif /* !ONE_TIME_STEP */
#endif /* !TWO_TIME_STEPS */

#if defined (GRID_1D)
          ParallelGridCoordinate pos (i);
#endif
#if defined (GRID_2D)
          ParallelGridCoordinate pos (i, j);
#endif
#if defined (GRID_3D)
          ParallelGridCoordinate pos (i, j, k);
#endif
          setFieldPointValue (val, ParallelGridCoordinate (pos));

#if defined (GRID_3D)
        }
#endif
#if defined (GRID_2D) || defined (GRID_3D)
      }
#endif
#if defined (GRID_1D) || defined (GRID_2D) || defined (GRID_3D)
    }
#endif
  }
}

#endif /* PARALLEL_GRID */