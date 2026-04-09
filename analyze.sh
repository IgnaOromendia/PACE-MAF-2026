rm -rf edgeScore/*
sh ./strideRunner.sh 20leaves_2trees
python3 process_edge_scores.py