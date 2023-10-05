function trace
    echo $argv
    eval $argv
end

function rebuild
    if not test -d build
        echo "creating build directory"
        trace mkdir build
    end

    if contains debug $argv
        set build_type Debug
    else
        set bulid_type Release
    end

    trace cd build
    trace cmake -DCMAKE_BUILD_TYPE=$build_type ..
    trace cd ..
end

function remake
    trace cd build
    trace make
    trace cd ..
end

echo "added remake command"
echo "added rebuild command"
