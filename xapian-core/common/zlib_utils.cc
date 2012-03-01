#include <config.h>

#include "zlib_utils.h"
#include "str.h"
#include "stringutils.h"

CompressionStream::CompressionStream(int compress_strategy_)
    : compress_strategy(compress_strategy_),
      zerr(0),
      out_len(0),
      out(NULL),
      deflate_zstream(NULL),
      inflate_zstream(NULL)
{
    // LOGCALL_CTOR()
}

CompressionStream::~CompressionStream() {
    if (deflate_zstream) {
	// Errors which we care about have already been handled, so just ignore
	// any which get returned here.
	(void) deflateEnd(deflate_zstream);
	delete deflate_zstream;
    }

    if (inflate_zstream) {
	// Errors which we care about have already been handled, so just ignore
	// any which get returned here.
	(void) inflateEnd(inflate_zstream);
	delete inflate_zstream;
    }
    
    if (out) {
	delete [] out;
    }
}


void
CompressionStream::compress(string & buf) {
    out_len = buf.size() - 1;
    out = new unsigned char[out_len];
    deflate_zstream->avail_in = (uInt)buf.size();
    deflate_zstream->next_in = (Bytef *)const_cast<char *>(buf.data());
    deflate_zstream->next_out = out;
    deflate_zstream->avail_out = (uInt)out_len;
    zerr = deflate(deflate_zstream, Z_FINISH);
}


void
CompressionStream::compress(byte * buf, int size) {
    out_len = size - 1;
    out = new unsigned char[out_len];
    deflate_zstream->avail_in = (uInt)size;
    deflate_zstream->next_in = (Bytef *)(buf);
    deflate_zstream->next_out = out;
    deflate_zstream->avail_out = (uInt)out_len;
    zerr = deflate(deflate_zstream, Z_FINISH);
}

// void
// CompressionStream::decompress(string & buf) {
//     inflate_zstream->next_in = (Bytef*)const_cast<char *>(tag->data());
//     inflate_zstream->avail_in = (uInt)tag->size();
//     int err = Z_OK;
//     while (err != Z_STREAM_END) {
// 	comp_stream.inflate_zstream->next_out = buf;
// 	comp_stream.inflate_zstream->avail_out = (uInt)sizeof(buf);
// 	err = inflate(comp_stream.inflate_zstream, Z_SYNC_FLUSH);
// 	if (err == Z_BUF_ERROR && comp_stream.inflate_zstream->avail_in == 0) {
// 	    LOGLINE(DB, "Z_BUF_ERROR - faking checksum of " << comp_stream.inflate_zstream->adler);
// 	    Bytef header2[4];
// 	    setint4(header2, 0, comp_stream.inflate_zstream->adler);
// 	    comp_stream.inflate_zstream->next_in = header2;
// 	    comp_stream.inflate_zstream->avail_in = 4;
// 	    err = inflate(comp_stream.inflate_zstream, Z_SYNC_FLUSH);
// 	    if (err == Z_STREAM_END) break;
// 	}

// 	if (err != Z_OK && err != Z_STREAM_END) {
// 	    if (err == Z_MEM_ERROR) throw std::bad_alloc();
// 	    string msg = "inflate failed";
// 	    if (comp_stream.inflate_zstream->msg) {
// 		msg += " (";
// 		msg += comp_stream.inflate_zstream->msg;
// 		msg += ')';
// 	    }
// 	    throw Xapian::DatabaseError(msg);
// 	}

// 	utag.append(reinterpret_cast<const char *>(buf),
// 		    comp_stream.inflate_zstream->next_out - buf);
//     }
//     if (utag.size() != comp_stream.inflate_zstream->total_out) {
// 	string msg = "compressed tag didn't expand to the expected size: ";
// 	msg += str(utag.size());
// 	msg += " != ";
// 	// OpenBSD's zlib.h uses off_t instead of uLong for total_out.
// 	msg += str((size_t)comp_stream.inflate_zstream->total_out);
// 	throw Xapian::DatabaseCorruptError(msg);
//     }
// }


void
CompressionStream::lazy_alloc_deflate_zstream() const {
    if (usual(deflate_zstream)) {
	if (usual(deflateReset(deflate_zstream) == Z_OK)) return;
	// Try to recover by deleting the stream and starting from scratch.
	delete deflate_zstream;
    }

    deflate_zstream = new z_stream;

    deflate_zstream->zalloc = reinterpret_cast<alloc_func>(0);
    deflate_zstream->zfree = reinterpret_cast<free_func>(0);
    deflate_zstream->opaque = (voidpf)0;

    // -15 means raw deflate with 32K LZ77 window (largest)
    // memLevel 9 is the highest (8 is default)
    int err;
    // FIXME:dc: this needs to really use compress_strategy if set
    err = deflateInit2(deflate_zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
		       -15, 9, Z_DEFAULT_STRATEGY);
    if (rare(err != Z_OK)) {
	if (err == Z_MEM_ERROR) {
	    delete deflate_zstream;
	    deflate_zstream = 0;
	    throw std::bad_alloc();
	}
	string msg = "deflateInit2 failed (";
	if (deflate_zstream->msg) {
	    msg += deflate_zstream->msg;
	} else {
	    msg += str(err);
	}
	msg += ')';
	delete deflate_zstream;
	deflate_zstream = 0;
	throw Xapian::DatabaseError(msg);
    }
}

void
CompressionStream::lazy_alloc_inflate_zstream() const {
    if (usual(inflate_zstream)) {
	if (usual(inflateReset(inflate_zstream) == Z_OK)) return;
	// Try to recover by deleting the stream and starting from scratch.
	delete inflate_zstream;
    }

    inflate_zstream = new z_stream;

    inflate_zstream->zalloc = reinterpret_cast<alloc_func>(0);
    inflate_zstream->zfree = reinterpret_cast<free_func>(0);

    inflate_zstream->next_in = Z_NULL;
    inflate_zstream->avail_in = 0;

    int err = inflateInit2(inflate_zstream, -15);
    if (rare(err != Z_OK)) {
	if (err == Z_MEM_ERROR) {
	    delete inflate_zstream;
	    inflate_zstream = 0;
	    throw std::bad_alloc();
	}
	string msg = "inflateInit2 failed (";
	if (inflate_zstream->msg) {
	    msg += inflate_zstream->msg;
	} else {
	    msg += str(err);
	}
	msg += ')';
	delete inflate_zstream;
	inflate_zstream = 0;
	throw Xapian::DatabaseError(msg);
    }
}
