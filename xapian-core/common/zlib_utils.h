
#include <zlib.h>

#include "xapian/error.h"

#include "debuglog.h"
#include "utils.h"

using namespace std;

#define DONT_COMPRESS -1

class CompressionStream {

 public:
    CompressionStream(int);

    ~CompressionStream();

    int compress_strategy;

    /// Zlib state object for deflating
    mutable z_stream *deflate_zstream;

    /// Zlib state object for inflating
    mutable z_stream *inflate_zstream;

    /// Allocate the zstream for deflating, if not already allocated.
    void lazy_alloc_deflate_zstream() const;

    /// Allocate the zstream for inflating, if not already allocated.
    void lazy_alloc_inflate_zstream() const;
};
