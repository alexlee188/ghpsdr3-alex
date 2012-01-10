/*
 *   buffer.h
 *
 */

// g0orx binary header
#define BUFFER_HEADER_SIZE 15

// 0     buffer type
// 1     header version
// 2     header subversion
// 3-4   samples length
// 5-6   main rx meter
// 7-8   sub rx meter
// 9-12  sample rate
// 13-14 IF         (added in version 2.1)

#define AUDIO_BUFFER_HEADER_SIZE 5

// 0     buffer type
// 1     header version
// 2     header subversion
// 3-4   samples length

#define SPECTRUM_BUFFER     0
#define AUDIO_BUFFER        1
#define BANDSCOPE_BUFFER    2
#define RTP_REPLY_BUFFER    3
#define ANSWER_BUFFER    	4

// g0orx binary header
#define HEADER_VERSION  2
#define HEADER_SUBVERSION  1
