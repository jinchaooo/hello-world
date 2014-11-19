# dump crush map to file
ceph osd getcrushmap -o cm_bin


# decompile binary crush map to txt
crushtool -d cm_bin -o cm_txt


# edit txt crush map
vim cm_txt


# compile txt crush map to binary
crushtool  --enable-unsafe-tunables -c cm_txt -o cm_bin_new


# set updated crush map from binary
ceph osd  setcrushmap -i cm_bin_new
