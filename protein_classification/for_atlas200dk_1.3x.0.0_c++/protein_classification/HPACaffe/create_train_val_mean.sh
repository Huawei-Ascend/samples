#!/usr/bin/env sh
# Compute the mean image from the imagenet training lmdb
# N.B. this is available in data/ilsvrc12

EXAMPLE=/home/leihouchao/leihouchao/caffeHuaWei/lmdb_data/
DATA=/home/leihouchao/leihouchao/caffeHuaWei/lmdb_data/
TOOLS=/home/leihouchao/caffe/build/tools/

echo "\nprocess train mean lmdb..."
$TOOLS/compute_image_mean $EXAMPLE/train_lmdb \
  $DATA/train_mean.binaryproto


echo "\nprocess test mean lmdb..."
$TOOLS/compute_image_mean $EXAMPLE/test_lmdb \
  $DATA/test_mean.binaryproto

echo ""
echo "Done."
