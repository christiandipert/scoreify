#include <stdio.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <unistd.h>
#include <lame/lame.h>

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define SECONDS_TO_RECORD 10
#define MP3_BUFFER_SIZE 8192

// Global LAME encoder
static lame_global_flags *gfp = NULL;

// Forward declaration
static void writeAudioBuffer(AudioQueueBufferRef inBuffer);

/*!
    @brief Audio queue callback function
    @param inUserData User data pointer
    @param inAQ Audio queue reference
    @param inBuffer Audio queue buffer reference
    @param inStartTime Audio time stamp
    @param inNumPackets Number of packets
    @param inPacketDesc Audio stream packet description
*/
static void HandleInputBuffer(void *inUserData,
                            AudioQueueRef inAQ,
                            AudioQueueBufferRef inBuffer,
                            const AudioTimeStamp *inStartTime,
                            UInt32 inNumPackets,
                            const AudioStreamPacketDescription *inPacketDesc) {
    // Write buffer & enqueue
    writeAudioBuffer(inBuffer);
    AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
}

/*!
    @brief Write the audio buffer to a file
    @param inBuffer Audio queue buffer reference
*/
static void writeAudioBuffer(AudioQueueBufferRef inBuffer) {
    static FILE *mp3File = NULL;
    static bool isFirstWrite = true;
    
    // Open MP3 file on first write
    if (isFirstWrite) {
        mp3File = fopen("recording.mp3", "wb");
        if (!mp3File) {
            printf("Error opening MP3 file\n");
            return;
        }
        isFirstWrite = false;
    }
    
    if (mp3File) {
        // Convert the buffer to 16-bit samples
        short *samples = (short *)inBuffer->mAudioData;
        int numSamples = inBuffer->mAudioDataByteSize / sizeof(short);
        
        // Create MP3 buffer
        unsigned char mp3Buffer[MP3_BUFFER_SIZE];
        
        // Encode to MP3
        int bytesWritten = lame_encode_buffer_interleaved(
            gfp,
            samples,
            numSamples / CHANNELS,
            mp3Buffer,
            MP3_BUFFER_SIZE
        );
        
        if (bytesWritten > 0) {
            fwrite(mp3Buffer, 1, bytesWritten, mp3File);
        }
    }
}

/*!
    @brief Main driver of the program
    @return int 0 on success, non-zero on failure
*/
int main() {
    // Initialize LAME encoder
    gfp = lame_init();
    if (!gfp) {
        printf("Error initializing LAME encoder\n");
        return 1;
    }
    
    // Set LAME parameters
    lame_set_num_channels(gfp, CHANNELS);
    lame_set_in_samplerate(gfp, SAMPLE_RATE);
    lame_set_brate(gfp, 128);  // 128kbps bitrate
    lame_set_quality(gfp, 5);  // Quality setting (0-9, 0 is best)
    
    // Initialize the encoder
    if (lame_init_params(gfp) < 0) {
        printf("Error initializing LAME parameters\n");
        lame_close(gfp);
        return 1;
    }

    AudioStreamBasicDescription audioFormat;
    audioFormat.mSampleRate = SAMPLE_RATE;
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    audioFormat.mBitsPerChannel = 16;
    audioFormat.mChannelsPerFrame = CHANNELS;
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mBytesPerFrame = audioFormat.mBitsPerChannel / 8 * audioFormat.mChannelsPerFrame;
    audioFormat.mBytesPerPacket = audioFormat.mBytesPerFrame * audioFormat.mFramesPerPacket;
    
    AudioQueueRef queue;
    AudioQueueNewInput(&audioFormat,
                      HandleInputBuffer,
                      NULL,
                      NULL,
                      NULL,
                      0,
                      &queue);
    
    const int NUM_BUFFERS = 3;
    const int BUFFER_SIZE = 4096;
    AudioQueueBufferRef buffers[NUM_BUFFERS];
    
    for (int i = 0; i < NUM_BUFFERS; i++) {
        AudioQueueAllocateBuffer(queue, BUFFER_SIZE, &buffers[i]);
        AudioQueueEnqueueBuffer(queue, buffers[i], 0, NULL);
    }
    
    printf("Recording for %d seconds...\n", SECONDS_TO_RECORD);
    AudioQueueStart(queue, NULL);
    
    sleep(SECONDS_TO_RECORD);
    
    AudioQueueStop(queue, true);
    AudioQueueDispose(queue, true);
    
    // Write the MP3 header and close the encoder
    if (gfp) {
        unsigned char mp3Buffer[MP3_BUFFER_SIZE];
        int bytesWritten = lame_encode_flush(gfp, mp3Buffer, MP3_BUFFER_SIZE);
        if (bytesWritten > 0) {
            FILE *mp3File = fopen("recording.mp3", "ab");
            if (mp3File) {
                fwrite(mp3Buffer, 1, bytesWritten, mp3File);
                fclose(mp3File);
            }
        }
        lame_close(gfp);
    }
    
    printf("Recording complete! Saved to recording.mp3\n");
    return 0;
}
