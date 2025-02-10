# Rady k tomuto projektu
## Rady k rychlosti:
- pokud si chcete porovnat výsledky s mými můžete pustit `sbatch bench.sl`.
  - pamatuj však že překladač dělá vělké rozdíly a výsledky se mohou lišit (v moje roce Intel co je specifikován v zadání)
  - moje výsledky tohoto script jsou:
```sql
iterations: 20
builder: loop
input: ../data/bun_zipper_res3
threads: 18, avg: 478 ms
threads: 36, avg: 240 ms
iterations: 20
builder: loop
input: ../data/bun_zipper_res2
threads: 18, avg: 2457 ms
threads: 36, avg: 1219 ms
iterations: 20
builder: loop
input: ../data/bun_zipper_res4
threads: 18, avg: 148 ms
threads: 36, avg: 75 ms
iterations: 20
builder: loop
input: ../data/bun_zipper_res1
threads: 18, avg: 11988 ms
threads: 36, avg: 5997 ms
iterations: 20
builder: tree
input: ../data/bun_zipper_res3
threads: 18, avg: 170 ms
threads: 36, avg: 103 ms
iterations: 20
builder: tree
input: ../data/bun_zipper_res2
threads: 18, avg: 854 ms
threads: 36, avg: 437 ms
iterations: 20
builder: tree
input: ../data/bun_zipper_res4
threads: 18, avg: 71 ms
threads: 36, avg: 89 ms
iterations: 20
builder: tree
input: ../data/bun_zipper_res1
threads: 18, avg: 4183 ms
threads: 36, avg: 2133 ms
  ```
*poznamka: Nepoužívám SIMD protože je zakázáno zadáním ale v kodu vidíte kde bych je dal (výsledek je asi o 15-30% lepší)* ale v moje roku za to nebyli strhávany body :))

## Spočítat čas co jsem spotřeboval na barci
```bash
sacct --starttime 2024-11-20 -p | awk -F'|' '$1 ~ /^[0-9]+$/ {split($9, t, ":"); total += t[1]*3600 + t[2]*60 + t[3]; count++} END {printf "Celkem čas: %02d:%02d:%02d\nCelkem řádků: %d\n", total/3600, (total%3600)/60, total%60, count}'
```

## Jak spočítat tu efektivitu
`perf.sl` by ti měl dát odpoveď na tuto otázku.

## Zajímave věci

koukněte na soubor `AVS_Projekt 2.pdf` jsou tam něják typické chyby :)

