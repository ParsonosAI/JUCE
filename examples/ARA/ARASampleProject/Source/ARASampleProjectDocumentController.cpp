#include "ARASampleProjectDocumentController.h"

ARA::PlugIn::AudioModification* ARASampleProjectDocumentController::doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
{
    return new ARASampleProjectAudioModification (static_cast<ARAAudioSource*> (audioSource), hostRef, static_cast<const ARAAudioModification*> (optionalModificationToClone));
}

bool ARASampleProjectDocumentController::doRestoreObjectsFromStream (ARAInputStream& input, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept
{
    // start reading data from the archive, starting with the number of audio modifications in the archive
    const auto numAudioModifications = input.readInt64();

    // loop over stored audio modification data
    for (int64 i = 0; i < numAudioModifications; ++i)
    {
        const float progressVal = static_cast<float> (i) / static_cast<float> (numAudioModifications);
        getHostArchivingController()->notifyDocumentUnarchivingProgress (progressVal);

        // read audio modification persistent ID and analysis result from archive
        const String persistentID = input.readString();
        const bool reverse = input.readBool();

        // find audio modification to restore the state to (drop state if not to be loaded)
        auto audioModification = filter->getAudioModificationToRestoreStateWithID<ARASampleProjectAudioModification> (persistentID.getCharPointer());
        if (!audioModification)
            continue;

        audioModification->setReversePlayback (reverse);
    }

    getHostArchivingController()->notifyDocumentUnarchivingProgress (1.0f);

    return ! input.failed();
}

bool ARASampleProjectDocumentController::doStoreObjectsToStream (ARAOutputStream& output, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept
{
    // this dummy implementation only deals with audio modification states
    const auto& audioModificationsToPersist{ filter->getAudioModificationsToStore<ARASampleProjectAudioModification>() };

    // write the number of audio modifications we are persisting
    const size_t numAudioModifications = audioModificationsToPersist.size();
    bool success = output.writeInt64 ((int64)numAudioModifications);

    // for each audio modification to persist, persist its ID followed by whether or not it's reversed
    for (size_t i = 0; i < numAudioModifications; ++i)
    {
        // write persistent ID and reverse state
        success = success && output.writeString (audioModificationsToPersist[i]->getPersistentID());
        success = success && output.writeBool (audioModificationsToPersist[i]->getReversePlayback());

        const float progressVal = static_cast<float> (i) / static_cast<float> (numAudioModifications);
        getHostArchivingController()->notifyDocumentArchivingProgress (progressVal);
    }

    getHostArchivingController()->notifyDocumentArchivingProgress (1.0);

    return success;
}

//==============================================================================
// Hook defined by the ARA SDK to create custom subclass
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController (const ARA::ARADocumentControllerHostInstance* instance) noexcept
{
    return new ARASampleProjectDocumentController (instance);
}
