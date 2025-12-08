# Flash attention example

Example demonstrates the implementation of flash attention (scaled dot product attention) in TinyTC.
The Q, K, V, O tensor dimension is given by DxTxNxB and controlled by the -d, -t, -n, -b switches, respectively.
Supported data types are float16 (-ff16) and bfloat16 (-fbf16).

Please set
```
export IGC_EnableSelectiveScalarizer=1
export NEO_CACHE_PERSISTENT=0
```
for best performance. 
