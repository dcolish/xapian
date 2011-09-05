CompressionStream:CompressionStream()
    : deflate_zstream(NULL),
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
}

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
    err = deflateInit2(deflate_zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
		       -15, 9, compress_strategy);
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
