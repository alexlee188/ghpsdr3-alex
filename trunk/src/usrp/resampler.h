/**
* @file resampler.h
* @brief Sampling rate alteration service
* @author Alberto Trentadue, IZ0CEZ
* @version 0.1
* @date 2012-02-20
*/

#define MAX_RESAMPLERS 4
#define RRESAMPLER_NO_ERROR 0
#define NO_MORE_RESAMPLERS -1
#define FAILED_RESAMPLER -2

typedef struct _resampinfo {
    bool used;
    int max_frames;
    int interp;
    int decim;
} RESAMPINFO;

/*!
 * MUST be called ONLY ONCE! prior to any subsequent usage
 */ 
void resampler_init(void);

/*!
 * Prepares a new resampler control data object.
 * Params are: the input vector size per channel (2 channels), 
 * the decimation and interpolation values, so 
 * that resampling ratio=interp/decim.
 * Thread safe function.
 * Returns the resampler unique id in a int.
 */
int resampler_setup_new(int max_frames, int decim, int interp);

/*!
 * Releases a resampler control data object
 */ 
void release_resampler(int r_id);

/*!
 * Loads the input data (2 channels), up to the defined size
 * in the identified resampler.
 * Size limit check is CALLER responsibility
 */
void resampler_load_channels(int r_id, float *ch1, float *ch2);


/*!
 * Loads the individual input data (2 channels), up to the defined size
 * in the identified resampler.
 * Size limit check is CALLER responsibility
 */
void resampler_load_data(int r_id, int idx, float ch1, float ch2);

/*!
 * Executes the resampling on a given set of data
 */
int do_resample(int r_id, int frames, int *out_frames_gen, const char *message);


/*!
 * Fetches the 2 resampled vectors as separate arrays 
 * one element by one.
 * Size limit check is CALLER responsibility
 */
void resampler_fetch_data(int r_id, int idx, float * ch1, float * ch2);

/*    
void resampler_copy_data_in(int r_id, int frames, float *copy);

SRC_DATA * get_resampler_src_data(int r_id);

void set_resampler_src_data(int r_id, SRC_DATA * data);
*/