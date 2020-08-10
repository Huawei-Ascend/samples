# 模型转换
# 删除旧东西
rm -rf out/*
rm -rf data/test.*

# hpa 模型
atc --model=caffe_model/hpa.prototxt \
    --weight=caffe_model/hpa.caffemodel \
    --framework=0 \
    --output=model/deploy_vel \
    --soc_version=Ascend310 \
    --input_format=NCHW \
    --input_fp16_nodes=data \
    -output_type=FP32 \
    --out_nodes="score:0"

# 编译
cd build/intermediates/host
cmake ../../../src -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE
make

# 修改main的权限
cd ../../../out 
chmod 777 main 

# 运行
cd ..
python3.7.5 server.py