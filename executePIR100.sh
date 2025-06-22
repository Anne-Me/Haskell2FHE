#!/bin/bash

# Default KeyGen
./keygen

# Create dummy database of size 100 that has integers 0 to 99 as entries
for i in {0..99}; do
  ./build/encrypt -b 32 -n 1 "$i" -key secret.key -prefixout "ct_db_$i" 
done

# Query
./build/encrypt -b 32 -n 1 5 -key secret.key -prefixout "ct_query" 

# Evaluate homomorphically using 5 threads
./build/clash2tfhe -c tests/programs/PIR/PIR100.json -n 101 ct_query_0.data ct_db_0_0.data ct_db_1_0.data ct_db_2_0.data ct_db_3_0.data ct_db_4_0.data ct_db_5_0.data ct_db_6_0.data ct_db_7_0.data ct_db_8_0.data ct_db_9_0.data ct_db_10_0.data ct_db_11_0.data ct_db_12_0.data ct_db_13_0.data ct_db_14_0.data ct_db_15_0.data ct_db_16_0.data ct_db_17_0.data ct_db_18_0.data ct_db_19_0.data ct_db_20_0.data ct_db_21_0.data ct_db_22_0.data ct_db_23_0.data ct_db_24_0.data ct_db_25_0.data ct_db_26_0.data ct_db_27_0.data ct_db_28_0.data ct_db_29_0.data ct_db_30_0.data ct_db_31_0.data ct_db_32_0.data ct_db_33_0.data ct_db_34_0.data ct_db_35_0.data ct_db_36_0.data ct_db_37_0.data ct_db_38_0.data ct_db_39_0.data ct_db_40_0.data ct_db_41_0.data ct_db_42_0.data ct_db_43_0.data ct_db_44_0.data ct_db_45_0.data ct_db_46_0.data ct_db_47_0.data ct_db_48_0.data ct_db_49_0.data ct_db_50_0.data ct_db_51_0.data ct_db_52_0.data ct_db_53_0.data ct_db_54_0.data ct_db_55_0.data ct_db_56_0.data ct_db_57_0.data ct_db_58_0.data ct_db_59_0.data ct_db_60_0.data ct_db_61_0.data ct_db_62_0.data ct_db_63_0.data ct_db_64_0.data ct_db_65_0.data ct_db_66_0.data ct_db_67_0.data ct_db_68_0.data ct_db_69_0.data ct_db_70_0.data ct_db_71_0.data ct_db_72_0.data ct_db_73_0.data ct_db_74_0.data ct_db_75_0.data ct_db_76_0.data ct_db_77_0.data ct_db_78_0.data ct_db_79_0.data ct_db_80_0.data ct_db_81_0.data ct_db_82_0.data ct_db_83_0.data ct_db_84_0.data ct_db_85_0.data ct_db_86_0.data ct_db_87_0.data ct_db_88_0.data ct_db_89_0.data ct_db_90_0.data ct_db_91_0.data ct_db_92_0.data ct_db_93_0.data ct_db_94_0.data ct_db_95_0.data ct_db_96_0.data ct_db_97_0.data ct_db_98_0.data ct_db_99_0.data  -b 32 -boot boots.key -out result.data -t 5

# Decrypt result
./build/decrypt -b 1 -key secret.key result.data