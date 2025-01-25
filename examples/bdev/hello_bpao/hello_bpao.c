/*   SPDX-License-Identifier: BSD-3-Clause
 *   Copyright (C) 2018 Intel Corporation.
 *   All rights reserved.
 */

#include "spdk/bdev.h"
#include "spdk/bdev_zone.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/ftl.h"
#include "spdk/log.h"
#include "spdk/stdinc.h"
#include "spdk/string.h"
#include "spdk/thread.h"

static char *g_bdev_name = "Malloc0";
/*
 * We'll use this struct to gather housekeeping hello_context to pass between
 * our events and callbacks.
 */
struct hello_context_t {
  struct spdk_ftl_dev *dev;
  struct spdk_io_channel *io_channel;
  uint32_t buff1_size;
  uint32_t buff2_size;
  char *bdev_name;
  struct iovec *iov;
  struct spdk_bdev_io_wait_entry bdev_io_wait;
};

struct ftl_io;

/*
 * Usage function for printing parameters that are specific to this application
 */
static void hello_bdev_usage(void) {
  printf(" -b <bdev>                 name of the bdev to use\n");
}

/*
 * This function is called to parse the parameters that are specific to this
 * application
 */
static int hello_bdev_parse_arg(int ch, char *arg) {
  switch (ch) {
    case 'b':
      g_bdev_name = arg;
      break;
    default:
      return -EINVAL;
  }
  return 0;
}

/*
 * Callback function for read io completion.
 */
// static void read_complete(struct spdk_bdev_io *bdev_io, bool success,
//                           void *cb_arg) {
//   struct hello_context_t *hello_context = cb_arg;

//   if (success) {
//     SPDK_NOTICELOG("Read string from bdev : %s\n", hello_context->buff);
//   } else {
//     SPDK_ERRLOG("bdev io read error\n");
//   }

//   /* Complete the bdev io and close the channel */
//   spdk_bdev_free_io(bdev_io);
//   spdk_put_io_channel(hello_context->bdev_io_channel);
//   spdk_bdev_close(hello_context->bdev_desc);
//   SPDK_NOTICELOG("Stopping app\n");
//   spdk_app_stop(success ? 0 : -1);
// }

// static void hello_read(void *arg) {
//   struct hello_context_t *hello_context = arg;
//   int rc = 0;

//   SPDK_NOTICELOG("Reading io\n");
//   rc = spdk_bdev_read(hello_context->bdev_desc,
//   hello_context->bdev_io_channel,
//                       hello_context->buff, 0, hello_context->buff_size,
//                       read_complete, hello_context);

//   if (rc == -ENOMEM) {
//     SPDK_NOTICELOG("Queueing io\n");
//     /* In case we cannot perform I/O now, queue I/O */
//     hello_context->bdev_io_wait.bdev = hello_context->bdev;
//     hello_context->bdev_io_wait.cb_fn = hello_read;
//     hello_context->bdev_io_wait.cb_arg = hello_context;
//     spdk_bdev_queue_io_wait(hello_context->bdev,
//     hello_context->bdev_io_channel,
//                             &hello_context->bdev_io_wait);
//   } else if (rc) {
//     SPDK_ERRLOG("%s error while reading from bdev: %d\n", spdk_strerror(-rc),
//                 rc);
//     spdk_put_io_channel(hello_context->bdev_io_channel);
//     spdk_bdev_close(hello_context->bdev_desc);
//     spdk_app_stop(-1);
//   }
// }

/*
 * Callback function for write io completion.
 */
// static void write_complete(struct spdk_bdev_io *bdev_io, bool success,
//                            void *cb_arg) {
//   struct hello_context_t *hello_context = cb_arg;

//   /* Complete the I/O */
//   spdk_bdev_free_io(bdev_io);

//   if (success) {
//     SPDK_NOTICELOG("bdev io write completed successfully\n");
//   } else {
//     SPDK_ERRLOG("bdev io write error: %d\n", EIO);
//     spdk_put_io_channel(hello_context->bdev_io_channel);
//     spdk_bdev_close(hello_context->bdev_desc);
//     spdk_app_stop(-1);
//     return;
//   }

//   /* Zero the buffer so that we can use it for reading */
//   memset(hello_context->buff, 0, hello_context->buff_size);

//   hello_read(hello_context);
// }

void write_cb(void *cb_arg, int status) {
  if (status) {
    SPDK_ERRLOG("ERROR writing to BPAO bdev\n");
    spdk_app_stop(-1);
    abort();
  }
  SPDK_NOTICELOG("Finished writing to BPAO bdev\n");
}

static void hello_write(void *arg) {
  struct hello_context_t *hello_context = arg;
  int rc = 0;

  SPDK_NOTICELOG("Writing to the bdev\n");

  // rc = spdk_bdev_write(hello_context->bdev_desc,
  // hello_context->bdev_io_channel, 		     hello_context->buff, 0,
  // hello_context->buff_size, write_complete, 		     hello_context);
  struct ftl_io *io = calloc(1, sizeof(io));

  rc = spdk_ftl_writev(hello_context->dev, io, hello_context->io_channel, 0, 2,
                       hello_context->iov, 2, write_cb, hello_context);
  if (rc != 0) {
    if (hello_context->iov[0].iov_base) {
      spdk_dma_free(hello_context->iov[0].iov_base);
    }
    if (hello_context->iov[1].iov_base) {
      spdk_dma_free(hello_context->iov[1].iov_base);
    }
    free(hello_context->iov);
    SPDK_ERRLOG("ERROR writing to BPAO bdev\n");
    spdk_app_stop(-1);
    abort();
  }
}

// static void
// hello_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
// 		    void *event_ctx)
// {
// 	SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
// }

// static void reset_zone_complete(struct spdk_bdev_io *bdev_io, bool success,
//                                 void *cb_arg) {
//   struct hello_context_t *hello_context = cb_arg;

//   /* Complete the I/O */
//   spdk_bdev_free_io(bdev_io);

//   if (!success) {
//     SPDK_ERRLOG("bdev io reset zone error: %d\n", EIO);
//     spdk_put_io_channel(hello_context->bdev_io_channel);
//     spdk_bdev_close(hello_context->bdev_desc);
//     spdk_app_stop(-1);
//     return;
//   }

//   hello_write(hello_context);
// }

// static void hello_reset_zone(void *arg) {
//   struct hello_context_t *hello_context = arg;
//   int rc = 0;

//   rc = spdk_bdev_zone_management(
//       hello_context->bdev_desc, hello_context->bdev_io_channel, 0,
//       SPDK_BDEV_ZONE_RESET, reset_zone_complete, hello_context);

//   if (rc == -ENOMEM) {
//     SPDK_NOTICELOG("Queueing io\n");
//     /* In case we cannot perform I/O now, queue I/O */
//     hello_context->bdev_io_wait.bdev = hello_context->bdev;
//     hello_context->bdev_io_wait.cb_fn = hello_reset_zone;
//     hello_context->bdev_io_wait.cb_arg = hello_context;
//     spdk_bdev_queue_io_wait(hello_context->bdev,
//     hello_context->bdev_io_channel,
//                             &hello_context->bdev_io_wait);
//   } else if (rc) {
//     SPDK_ERRLOG("%s error while resetting zone: %d\n", spdk_strerror(-rc),
//     rc); spdk_put_io_channel(hello_context->bdev_io_channel);
//     spdk_bdev_close(hello_context->bdev_desc);
//     spdk_app_stop(-1);
//   }
// }

void bpao_init_cb(struct spdk_ftl_dev *dev, void *cb_arg, int status) {
  if (status) {
    SPDK_ERRLOG("ERROR constructing BPAO bdev\n");
    spdk_app_stop(-1);
    abort();
  }
  SPDK_NOTICELOG("Finished constructing BPAO bdev\n");

  struct hello_context_t *hello_context = cb_arg;
  hello_context->dev = dev;
  hello_context->io_channel = spdk_ftl_get_io_channel(dev);

  /* Allocate a buffer for I/O */
  hello_context->iov = calloc(2, sizeof(struct iovec));
  if (!hello_context->iov) {
    SPDK_ERRLOG("Failed to allocate iov\n");
    spdk_app_stop(-1);
    return;
  }

  hello_context->buff1_size = 4096;
  hello_context->buff2_size = 4096;

  hello_context->iov[0].iov_base =
      spdk_dma_zmalloc(hello_context->buff1_size, 0, NULL);
  if (!hello_context->iov[0].iov_base) {
    SPDK_ERRLOG("Failed to allocate buffer\n");
    spdk_app_stop(-1);
    return;
  }
  hello_context->iov[0].iov_len = 4096;

  hello_context->iov[1].iov_base =
      spdk_dma_zmalloc(hello_context->buff2_size, 0, NULL);
  if (!hello_context->iov[1].iov_base) {
    SPDK_ERRLOG("Failed to allocate buffer\n");
    spdk_dma_free(hello_context->iov[0].iov_base);
    spdk_app_stop(-1);
    return;
  }
  hello_context->iov[1].iov_len = 4096;

  hello_write(hello_context);
}

/*
 * Our initial event that kicks off everything from main().
 */
static void hello_start(void *arg1) {
  struct hello_context_t *hello_context = arg1;
  int rc = 0;
  hello_context->dev = NULL;

  SPDK_NOTICELOG("Successfully started the application\n");

  SPDK_NOTICELOG("Now try to construct the BPAO bdev\n");
  struct spdk_ftl_conf conf;
  spdk_ftl_get_default_conf(&conf, sizeof(struct spdk_ftl_conf));
  conf.name = "bpao";
  conf.cache_bdev = "nvme1n1p0";
  conf.mode |= SPDK_FTL_MODE_CREATE;

  spdk_ftl_dev_init(&conf, bpao_init_cb, arg1);

  if (rc) {
    SPDK_ERRLOG("ERROR constructing BPAO bdev\n");
    spdk_app_stop(-1);
  }
}

int main(int argc, char **argv) {
  struct spdk_app_opts opts = {};
  int rc = 0;
  struct hello_context_t hello_context = {};

  /* Set default values in opts structure. */
  spdk_app_opts_init(&opts, sizeof(opts));
  opts.name = "hello_bdev";
  opts.rpc_addr = NULL;
  opts.json_config_file = "/dataset/spdk_config/nvme_ssd.json";

  /*
   * Parse built-in SPDK command line parameters as well
   * as our custom one(s).
   */
  if ((rc = spdk_app_parse_args(argc, argv, &opts, "b:", NULL,
                                hello_bdev_parse_arg, hello_bdev_usage)) !=
      SPDK_APP_PARSE_ARGS_SUCCESS) {
    exit(rc);
  }
  hello_context.bdev_name = g_bdev_name;

  /*
   * spdk_app_start() will initialize the SPDK framework, call hello_start(),
   * and then block until spdk_app_stop() is called (or if an initialization
   * error occurs, spdk_app_start() will return with rc even without calling
   * hello_start().
   */
  rc = spdk_app_start(&opts, hello_start, &hello_context);
  if (rc) {
    SPDK_ERRLOG("ERROR starting application\n");
  }

  /* At this point either spdk_app_stop() was called, or spdk_app_start()
   * failed because of internal error.
   */

  /* When the app stops, free up memory that we allocated. */

  /* Gracefully close out all of the SPDK subsystems. */
  spdk_app_fini();
  return rc;
}
