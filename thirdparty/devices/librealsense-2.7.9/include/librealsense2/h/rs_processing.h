/* License: Apache 2.0. See LICENSE file in root directory.
   Copyright(c) 2017 Intel Corporation. All Rights Reserved. */

/** \file rs2_processing.h
* \brief
* Exposes RealSense processing-block functionality for C compilers
*/


#ifndef LIBREALSENSE_RS2_PROCESSING_H
#define LIBREALSENSE_RS2_PROCESSING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rs_types.h"

/**
* Creates Depth-Colorizer processing block that can be used to quickly visualize the depth data
* This block will accept depth frames as input and replace them by depth frames with format RGB8
* Non-depth frames are passed through
* Further customization will be added soon (format, color-map, histogram equalization control)
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
*/
rs2_processing_block* rs2_create_colorizer(rs2_error** error);

/**
* Creates Sync processing block. This block accepts arbitrary frames and output composite frames of best matches
* Some frames may be released within the syncer if they are waiting for match for too long
* Syncronization is done (mostly) based on timestamps so good hardware timestamps are a pre-condition
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
*/
rs2_processing_block* rs2_create_sync_processing_block(rs2_error** error);

/**
* Creates Point-Cloud processing block. This block accepts depth frames and outputs Points frames
* In addition, given non-depth frame, the block will align texture coordinate to the non-depth stream
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
*/
rs2_processing_block* rs2_create_pointcloud(rs2_error** error);

/**
* This method creates new custom processing block. This lets the users pass frames between module boundaries for processing
* This is an infrastructure function aimed at middleware developers, and also used by provided blocks such as sync, colorizer, etc..
* \param ctx        RealSense context to query global parameters such as time
* \param proc       Processing function to be applied to every frame entering the block
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
* \return           new processing block, to be released by rs2_delete_processing_block
*/
rs2_processing_block* rs2_create_processing_block(rs2_frame_processor_callback* proc, rs2_error** error);

/**
* This method is used to direct the output from the processing block to some callback or sink object
* \param[in] block          Processing block
* \param[in] on_frame       Callback to be invoked every time the processing block calls frame_ready
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
*/
void rs2_start_processing(rs2_processing_block* block, rs2_frame_callback* on_frame, rs2_error** error);

/**
* This method is used to pass frame into a processing block
* \param[in] block          Processing block
* \param[in] frame          Frame to process, ownership is moved to the block object
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
*/
void rs2_process_frame(rs2_processing_block* block, rs2_frame* frame, rs2_error** error);

/**
* Deletes the processing block
* \param[in] block          Processing block
*/
void rs2_delete_processing_block(rs2_processing_block* block);

/**
* create frame queue. frame queues are the simplest x-platform synchronization primitive provided by librealsense
* to help developers who are not using async APIs
* \param[in] capacity max number of frames to allow to be stored in the queue before older frames will start to get dropped
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
* \return handle to the frame queue, must be released using rs2_delete_frame_queue
*/
rs2_frame_queue* rs2_create_frame_queue(int capacity, rs2_error** error);

/**
* deletes frame queue and releases all frames inside it
* \param[in] frame queue to delete
*/
void rs2_delete_frame_queue(rs2_frame_queue* queue);

/**
* wait until new frame becomes available in the queue and dequeue it
* \param[in] queue the frame queue data structure
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
* \return frame handle to be released using rs2_release_frame
*/
rs2_frame* rs2_wait_for_frame(rs2_frame_queue* queue, unsigned int timeout_ms, rs2_error** error);

/**
* poll if a new frame is available and dequeue if it is
* \param[in] queue the frame queue data structure
* \param[out] output_frame frame handle to be released using rs2_release_frame
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
* \return true if new frame was stored to output_frame
*/
int rs2_poll_for_frame(rs2_frame_queue* queue, rs2_frame** output_frame, rs2_error** error);

/**
* enqueue new frame into a queue
* \param[in] frame frame handle to enqueue (this operation passed ownership to the queue)
* \param[in] queue the frame queue data structure
*/
void rs2_enqueue_frame(rs2_frame* frame, void* queue);

/**
* Creates Align processing block. 
* \param[out] error  if non-null, receives any error that occurs during this call, otherwise, errors are ignored
*/
rs2_processing_block* rs2_create_align(rs2_stream align_to, rs2_error** error);

#ifdef __cplusplus
}
#endif
#endif
