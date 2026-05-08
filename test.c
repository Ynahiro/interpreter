int i = 1;
int result = 0;
int sq = 0;

do {
  result = result + i;
  sq = i * i;
  i = i + 1;
} while (i < 6);

printf(result);
printf(sq);
