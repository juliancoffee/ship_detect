function trace
    echo $argv
    eval $argv
end

function rebuild
    if not test -d build
        echo "creating build directory"
        trace mkdir build
    end

    trace cd build
    trace cmake ..
    trace cd ..
end

function remake
    trace cd build
    trace make
    trace cd ..
end

echo "added remake command"
echo "added rebuild command"
