for ((number=1;number<=400;number++))
{
  if(($number % 2 == 1))
  then
    ./voronoi_stippler -s ${number} -n -c -f -z 1 -p 5 ../hedcuter/images/solid-black-20.png output/solid-black_${number}.svg
  fi
}
