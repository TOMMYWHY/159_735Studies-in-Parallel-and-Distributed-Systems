### todo
# 
1. test gpgpu.pdf comd

export CUDA_HOME="/usr/local/cuda-10.0"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${CUDA_HOME}/lib64"
export PATH=${CUDA_HOME}/bin:${PATH}


nlenses:10; seq:177.921; cuda use 0.07
nlenses:40; seq:626.167; cuda use 0.073
nlenses:50; seq:756.878; cuda use 0.072
nlenses:80; seq:1475.05; cuda use 0.065
nlenses:100; seq:1452.27; cuda use 0.073
nlenses:150; seq:2317.29; cuda use 0.061
nlenses:200; seq:2918.42; cuda use 0.073
nlenses:250; seq:3700.34; cuda use 0.072
nlenses:300; seq:4400.07; cuda use 0.072
nlenses:350; seq:4943.4; cuda use 0.072
nlenses:400; seq:5641.99; cuda use 0.072
nlenses:450; seq:6315.74; cuda use 0.076
nlenses:500; seq:7044.67; cuda use 0.075
nlenses:550; seq:8075.2; cuda use 0.073
nlenses:600; seq:9736.85; cuda use 0.073
nlenses:650; seq:10461.3; cuda use 0.071
nlenses:700; seq:10001.6; cuda use 0.071
nlenses:750; seq:11738.5; cuda use 0.089
nlenses:800; seq:11394.2; cuda use 0.06
nlenses:850; seq:12485.7; cuda use 0.074
nlenses:900; seq:12921.6; cuda use 0.076
nlenses:950; seq:13797; cuda use 0.073
nlenses:1000; seq:14261.3; cuda use 0.072
nlenses:2000; seq:28333.6; cuda use 0.071
nlenses:3000; seq:42169.7; cuda use 0.075

