#%%
import numpy as np
import pandas as pd
from Bio import Phylo

# Načtení stromu ze souboru ve formátu Newick
tree = Phylo.read('tree.tre', 'newick')

# Vykreslení stromu
# Phylo.draw(tree)
print(tree)

from Bio import SeqIO

# Načtení všech sekvencí do seznamu
alignment = list(SeqIO.parse("msa.fasta", "fasta"))

#%% md
# # Task 1)
#%%
evolution_depth = {}


def rec_read(clade, depth=0):
    if clade.is_terminal():
        evolution_depth[clade.name] = depth
    else:
        return [rec_read(c, depth + 1) for c in clade.clades]


rec_read(tree.root)
print(evolution_depth, len(evolution_depth))
#%% md
# # Task 2)
# - normalizace
#%%
max_eval_distance = max(evolution_depth.values())
evolution_depth_normalized = {}

for name, depth in evolution_depth.items():
    evolution_depth_normalized[name] = depth / max_eval_distance

print(evolution_depth)
#%% md
# # Task 3)
# 
#%%
import numpy as np

# Převedeme sekvence na seznam řetězců
sequences = np.array([str(record.seq) for record in alignment])

alignment_length = 62

# Vytvoříme matici, kde každý prvek je seznam znaků z dané pozice
matrix = np.array([[s for s in seq] for seq in sequences])

#%%
order_of_sequences = [record.id for record in alignment]
output_fract = {}
score_of_conservacy = {}

amino_acids_order = list("ARNDCQEGHILKMFPSTWYV")

conservation_scores_df = pd.DataFrame(
    index=amino_acids_order,
    columns=range(1, alignment_length + 1),  # Sloupce číslované od 1
)

fract_top = 0
fract_bottom = 0

for column in range(alignment_length):
    column_data = matrix[:, column]
    fract_bottom  = 0
    fract_top = 0
    for acid in amino_acids_order:

        for i, sequence_symbol in enumerate(column_data):
            if sequence_symbol == acid:
                fract_top += evolution_depth_normalized[order_of_sequences[i]]
            fract_bottom += evolution_depth_normalized[order_of_sequences[i]]

        conservation_scores_df.loc[acid, column + 1] = fract_top / fract_bottom



conservation_scores_df
#%% md
# # Task 4)
# - load the aaindex file and normalization
#%%
def load_aaindex(file_path):
    properties = {}
    with open(file_path, 'r') as f:
        # Načteme všechny neprázdné řádky
        lines = [line.strip() for line in f if line.strip()]

    i = 0
    while i < len(lines):
        prop_name = lines[i]  # Název vlastnosti (např. "Hydrophobicity")
        header_line = lines[i + 1]  # Obsahuje páry např. "A/L     R/K     ..."
        row1 = lines[i + 2]  # První řádek číselných hodnot
        row2 = lines[i + 3]  # Druhý řádek číselných hodnot

        # Rozdělíme řádek se záhlavím na jednotlivé páry
        pairs = header_line.split()
        # Rozdělíme oba řádky na hodnoty a převedeme je na float
        values1 = [float(x) for x in row1.split()]
        values2 = [float(x) for x in row2.split()]

        # Vytvoříme slovník pro tuto vlastnost
        prop_dict = {}
        for pair, v1, v2 in zip(pairs, values1, values2):
            # Očekáváme formát "A/L", tedy oddělíme písmena
            aa1, aa2 = pair.split('/')
            prop_dict[aa1] = v1
            prop_dict[aa2] = v2

        properties[prop_name] = prop_dict
        i += 4  # posuneme se ke dalšímu bloku

    return properties


file_path = "aaindex.txt"  # cesta k vašemu souboru
aaindex_data = load_aaindex(file_path)

print(aaindex_data)

for features in aaindex_data.keys():

    # Vypíšeme hodnoty pro každou aminokyselinu ve vzestupném abecedním pořadí
    max_v = max(aaindex_data[features].values())
    min_v = min(aaindex_data[features].values())

    for k, v in aaindex_data[features].items():
        aaindex_data[features][k] = (v - min_v) / (max_v - min_v)

print()
for features in aaindex_data.keys():
    print(f"{features}:", end=" ")
    print(aaindex_data[features])
    print()
#%% md
# # Task 5)
#%%
import pandas as pd

physchem_property_names = list(aaindex_data.keys())

# Inicializujte výstupní DataFrame (20 řádků x N sloupců)
output_df = pd.DataFrame(
    index=amino_acids_order,
    columns=range(1, alignment_length + 1), # Sloupce číslované od 1 do N
    dtype=float # Ukládáme desetinná skóre
)


# Iterujte přes pozice v zarovnání (sloupce)
# Iteration over alignment positions (columns)
for j in range(alignment_length):
    # Získejte původní aminokyselinu na této pozici j z první (query) sekvence
    aa_original = sequences[0][j]

    # -- Speciální případ: Mezera v query sekvenci --
    if aa_original == '-':
        # Podle zadání, pokud je v query sekvenci na dané pozici mezera,
        # celý sloupec ve výstupu pro tuto pozici by měl být mezera (nebo NaN)
        # Poznámka: Pokud ukládáte jako CSV, '-' je lepší než NaN pro textové výstupy
        # Pokud byste chtěli NaN pro další numerické zpracování, použijte np.nan
        output_df[j + 1] = '-'
        continue # Přeskočte zbytek výpočtů pro tuto pozici a jděte na další sloupec

    # --- Nyní počítáme skóre pro všechny možné mutace na této pozici j ---

    # Získejte skóre konzervovanosti pro původní AA (aa_original) na pozici j
    # Indexy sloupců v conservation_scores_df jsou 1-based (j+1)
    cons_score_original = conservation_scores_df.loc[aa_original, j + 1]

    # Iterujte přes VŠECHNY STANDARDNÍ AMINOKYSELINY jako možné mutantní AA
    for aa_mutant in amino_acids_order:
        # --- Vypočítejte skóre mutace z aa_original na aa_mutant na pozici j ---

        mutation_score = 0.0 # Inicializujte skóre pro tuto konkrétní mutaci (suma)

        # Získejte skóre konzervovanosti pro mutantní AA (aa_mutant) na pozici j
        cons_score_mutant = conservation_scores_df.loc[aa_mutant, j + 1]

        # Iterujte přes VŠECHNY FYZIKÁLNĚ-CHEMICKÉ VLASTNOSTI (klíče ve slovníku aaindex_data)
        for property_name in physchem_property_names:
            # Získejte slovník hodnot pro tuto konkrétní vlastnost
            property_values = aaindex_data[property_name]

            # Získejte normalizovanou hodnotu této vlastnosti pro původní AA
            # Použijte .get() s defaultní hodnotou 0, pokud by náhodou AA v aaindexu chyběla
            # (i když by neměla, protože aaindex_data by měla obsahovat všech 20 AA)
            physchem_original_p = property_values.get(aa_original, 0.0)

            # Získejte normalizovanou hodnotu této vlastnosti pro mutantní AA
            physchem_mutant_p = property_values.get(aa_mutant, 0.0)

            # Vypočtěte člen součtu pro tuto vlastnost
            term_p = (cons_score_original * physchem_original_p) - (cons_score_mutant * physchem_mutant_p)

            # Přičtěte člen k celkovému skóre mutace
            mutation_score += term_p

        output_df.loc[aa_mutant, j + 1] = mutation_score



# add column with index
output_df.insert(0, 'AA', output_df.index)

output_df
#%%
