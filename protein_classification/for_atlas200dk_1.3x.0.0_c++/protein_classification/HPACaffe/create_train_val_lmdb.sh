echo "create train lmdb for resnet18..."
/home/leihouchao/caffe/build/tools/convert_imageset \
--resize_height=224 \
--resize_width=224 \
--backend="lmdb" \
--shuffle \
/home/leihouchao/leihouchao/caffeHuaWei/data/train/ \
/home/leihouchao/leihouchao/caffeHuaWei/data/train.txt \
/home/leihouchao/leihouchao/caffeHuaWei/lmdb_data/train_lmdb
echo "done"
echo ""
echo ""


echo "create val lmdb for resnet18..."
/home/leihouchao/caffe/build/tools/convert_imageset \
--resize_height=224 \
--resize_width=224 \
--backend="lmdb" \
--shuffle \
/home/leihouchao/leihouchao/caffeHuaWei/data/test/ \
/home/leihouchao/leihouchao/caffeHuaWei/data/test.txt \
/home/leihouchao/leihouchao/caffeHuaWei/lmdb_data/test_lmdb
echo "done"
echo ""
echo ""
