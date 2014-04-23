/*
 *  rf_playback_utils.h
 *
 *  Copyright (c) 2003 Reify Corp. All Rights reserved.
 */

#ifndef _RF_PLAYBACK_UTILS_H_
#define _RF_PLAYBACK_UTILS_H_

#include <vector>

// util
#include "rc_types.h"

// visual
#include "rc_framebuf.h"
#include "rc_videocache.h"

typedef struct rsPlayElement {
  rcSharedFrameBufPtr buf;
  uint32 duration;
};

extern bool rfGenerateTimeline(vector<uint32>& timeline,
			       double& msPerFrame,
			       rcVideoCache& cache);

#if 0
extern void rfGeneratePlaylist(vector<rsPlayElement>& playlist,
			       const int32 startFrame,
			       const int32 endFrame,
			       const int32 rateMultiplier,
			       const vector<int32>& timeline,
			       rcVideoCache& cache);
#endif
#endif
