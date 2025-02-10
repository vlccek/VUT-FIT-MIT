# Hodnocení 

```text
================================================================================
## 1: paralelizace puvodniho reseni (5b)
+1.00 b: spravnost vertices a distance: 517224 == 517224 a 0.00 mensi nez 2.0 (max 1 b)
+1.00 b: cas 157 ms do limitu 240 ms (a spravny vystup) (max 1 b)
+0.50 b: bonus za cas do 165 ms (bez SIMD) (max 0.5 b)
Detekovano SIMD: False
+1.00 b: Q1.1: kterou smycku paralelizovat (max 1 b)
+1.00 b: Q1.2: ukladani trojuhelniku (max 1 b)
+0.50 b: Q1.3: planovani (max 1 b)

## 2: paralelni pruchod stromem (8b)
+3.00 b: spravnost vertices a distance: 517224 == 517224 a 0.00 mensi nez 2.0 (max 3 b)
+2.00 b: cas 57 ms do limitu 100 ms (a spravny vystup) (max 2 b)
+0.50 b: bonus za cas do 70 ms (bez SIMD) (max 0.5 b)
+1.00 b: Q2.1: pouziti tasku (max 1 b)
+0.75 b: Q2.2: cut-off (max 1 b)
+1.00 b: Q2.3: ukladani trojuhelniku (max 1 b)
Detekovano SIMD: False

## 3: grafy skalovani, VTune (7b)
+1.00 b: Q3.1: efektivita skalovani (max 1 b)
+1.00 b: Q3.2: neefektivita prvni ulohy (max 1 b)
+1.00 b: Q3.3: tree efektivni pro slabe skalovani? (max 1 b)
+1.00 b: Q3.4: Slurm - prace na projektu (max 1 b)
+1.00 b: Q4.1: VTune - 18 jader (max 1 b)
+1.00 b: Q4.2: VTune - 36 jader (max 1 b)
+1.00 b: Q4.2: vypocet efektivity (max 1 b)

+0.00 b: bonus/penalta (max 0.0 b)
Celkem ziskano: 20.25 bodu

Komenar
======================
```

# Rady k tomuto projektu
## Rady k rychlosti
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


