for file in $(ls data/input/img)
do
    build/example data/input/img/$file --area 2000
done
