#!/bin/sh

run_prog() {
    preload="../testsuite/malloccheck.so"
    if ! test -r "$preload"
    then
        preload="../$preload"
    fi
    if ! test -r "$preload"
    then
        preload="../$preload"
    fi

    preload="$preload "`echo $preload | sed 's!malloccheck!alloccommon!'`

    if test "x$OM_NO_MALLOCCHECK" != "x"
    then
        preload=""
    fi
    
    OM_DTD_PATH="${srcdir}/../indexer/indexgraph/omindexer.dtd"
    export OM_DTD_PATH
    if test -r "$preload"
    then
        LD_PRELOAD="$preload"
	export LD_PRELOAD
	exec $USE_GDB "$@"
    else
        echo "malloccheck.so not found" >&2
	exec $USE_GDB "$@"
    fi
}

# this is a bit of a hack...
for x in * ; do
    case "$0" in 
        *run-$x) run_prog ./"$x" "$@" ;;
	*) ;;
    esac
done

echo "Failed to find test to run!" >&2
exit 1
