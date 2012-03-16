/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2011 Lawrence Livermore National Security, LLC
 * Description:   SAMRAI class to manage package startup and shutdown
 *
 ************************************************************************/

#ifndef included_tbox_SAMRAIManager
#define included_tbox_SAMRAIManager

#include "SAMRAI/SAMRAI_config.h"

#include "SAMRAI/tbox/Database.h"
#include "SAMRAI/tbox/SAMRAI_MPI.h"

namespace SAMRAI {
namespace tbox {

/*!
 * @brief Utility for managing startup and shutdown of SAMRAI objects
 * and for managing the maximum number of patch data components allowed.
 *
 * The startup/shutdown mechanism in SAMRAIManager is used to manage the
 * allocation and deallocation of certain memory, particularly static data
 * that must exist during the full extent of a run or for the full extent
 * of a single problem within a run.  The specific data that is controlled
 * by this mechanism is managed using the StartupShutdownManager.
 *
 * The four steps of the startup/shutdown mechanism are:
 *
 * <ul> initialize -- called at the start of a program after MPI is
 *      initialized but befor any other SAMRAI objects are used.
 * <ul> startup -- called to begin a problem-specific segment of the code.
 * <ul> shutdown -- called at the end of a problem-specific segment of the
 *                  code.  Shuts down and deallocates everything that was
 *                  started and allocated by startup.
 * <ul> finalize -- called at the end of a program right before MPI is
 *                  finalized.
 *
 * The startup and shutdown functions may be called multiple times within a
 * run, in order to allow for the execution of more than one problem within one
 * program.  Initialize and finalize must be called exactly once in a single
 * run.
 *
 * Additionally this class manages static data that controls the maximum
 * number of patch data components that can be allowed.
 *
 * @see tbox::SAMRAI_MPI
 * @see tbox::StartupShutdownManager
 */

class SAMRAIManager
{

public:
   /*!
    * @brief Initial setup of the SAMRAI package.
    *
    * This function should be invoked ONLY ONCE at the start of a process
    * to initialize SAMRAI and AFTER MPI is initialized (if used) by a
    * call to one of the SAMRAI_MPI init routines.
    *
    * This function initializes IEEE expection handlers and SAMRAI I/O, as
    * well as data for any classes that implement the initialize callback
    * interface through StartupShutdownManager.
    *
    * @param[in] setup_IEEE_assertion_handlers (defaults to true)
    */
   static void
   initialize(
      bool initialize_IEEE_assertion_handlers = true);

   /*!
    * @brief Startup of the SAMRAI package.
    *
    * This function invokes startup for any classes that implement the
    * startup callback interface through StartupShutdownManager.
    * This function may be invoked more than once in a process if
    * solving multiple SAMRAI problems.
    */
   static void
   startup()
   {
      TBOX_ASSERT(s_initialized);
      TBOX_ASSERT(!s_started);
      StartupShutdownManager::startup();
      s_started = true;
   }

   /*!
    * @brief Shutdown the SAMRAI package.
    *
    * This function invokes shutdown for any classes that implement the
    * startup callback interface through StartupShutdownManager.
    * This function may be invoked more than once in an process if
    * solving multiple SAMRAI problems.
    */
   static void
   shutdown()
   {
      TBOX_ASSERT(s_initialized);
      TBOX_ASSERT(s_started);
      StartupShutdownManager::shutdown();
      s_started = false;
   }

   /*!
    * @brief Final cleanup of the SAMRAI package.
    *
    * This function should be invoked ONLY ONCE at the end of a process
    * to complete the cleanup of SAMRAI memory allocations and
    * any other cleanup tasks.  IEEE assertion handlers and SAMRAI I/O
    * will be finalized, as well as data for any classes that implement
    * the finalize callback interface through StartupShutdownManager.
    *
    * After this function is called, the only thing that should occur before
    * exiting the program is a call to SAMRAI_MPI::finalize().
    *
    * This function should be invoked only once.
    */
   static void
   finalize()
   {
      TBOX_ASSERT(s_initialized);
      StartupShutdownManager::finalize();
      PIO::finalize();
      s_initialized = false;
   }

   /*!
    * @brief Returns true if SAMRAIManager has been initialized.
    */
   static bool
   isInitialized()
   {
      return s_initialized;
   }

   /*!
    * @brief Returns true if SAMRAIManager has been started.
    */
   static bool
   isStarted()
   {
      return s_started;
   }

   /*!
    * @brief Return maximum number of patch data entries supported by SAMRAI.
    *
    * The value is either the default value (256) or the value set by calling
    * the setMaxNumberPatchDataEntries() function.
    */
   static int
   getMaxNumberPatchDataEntries()
   {
      s_max_patch_data_entries_accessed = true;
      return s_max_patch_data_entries;
   }

   /*!
    * @brief Set maximum number of patch data entries supported by SAMRAI.
    *
    * The maximum number will be set to the maximum of the current value and
    * the argument value.
    *
    * Note that this routine cannot be called anytime after the max patch
    * data entries value has been accessed via the
    * getMaxNumberPatchDataEntries() function, neither by the user nor
    * internally within SAMRAI.  Typically, the first internal access of this
    * value occurs whenever any objects related to the patch hierarchy or
    * variables are created.
    */
   static void
   setMaxNumberPatchDataEntries(
      int maxnum);

private:
   /*!
    * Flag indicating SAMRAIManager has been initialized.
    */
   static bool s_initialized;

   /*!
    * Flag indicating startup has occured.
    */
   static bool s_started;

   /*!
    * Maximum number of patch data components allowed.
    */
   static int s_max_patch_data_entries;

   /*!
    * Flag telling whether maximum number of patch data components has
    * been accessed.
    */
   static bool s_max_patch_data_entries_accessed;

};

}
}

#endif
