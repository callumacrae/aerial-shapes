# Aerial shapes

An experiment in animations + shapes in aerial photos.

## To build

```
brew install cmake opencv boost
cmake .
make
```

(There's a good chance some of the dependencies will be installed already)

## To process assets

Images must be processed in advance (it takes a while if you have a lot of
images). Fill a directory with images and process them like this:

```
./out/process_images assets/test
```
