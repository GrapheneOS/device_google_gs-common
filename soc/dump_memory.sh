#!/vendor/bin/sh
echo "------ ION HEAPS ------"
for d in $(ls -d /d/ion/*)
do
  if [ -f $d ]; then
    echo --- $d
    cat $d
  else
    for f in $(ls $d)
    do
      echo --- $d/$f
      cat $d/$f
    done
  fi
done

echo "------ dmabuf info ------"
cat "/d/dma_buf/bufinfo"

echo "------ Page Pinner - longterm pin ------"
cat "/sys/kernel/debug/page_pinner/buffer"

