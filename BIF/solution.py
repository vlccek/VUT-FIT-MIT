#%%
"""
This python script is exported from python notebook it could contain unsuall lines of code that doesnt make sense in normal python script.
"""

from statistics import median

import numpy as np
import pandas as pd
from Bio import Phylo
from biorun.convert import false

# Načtení stromu ze souboru ve formátu Newick
tree = Phylo.read('tree.tre', 'newick')

# Vykreslení stromu
# Phylo.draw(tree)
print(tree)

from Bio import SeqIO

# loading the alignment
alignment = list(SeqIO.parse("msa.fasta", "fasta"))

#%% md
# # Task 1)
#%%
evolution_depth = {}
terminals = tree.get_terminals()  # all list nodes

for terminal_clade in terminals:
    distance = 0
    path_to_terminal = tree.get_path(terminal_clade)
    for clade_on_path in path_to_terminal:
        if clade_on_path.branch_length is not None:
            distance += clade_on_path.branch_length
    evolution_depth[terminal_clade.name] = distance

#%% md
# # Task 2)
# - normalization
#%%
max_eval_distance = max(evolution_depth.values())
min_eval_distance = min(evolution_depth.values())
# for min max normlization

evolution_depth_normalized = {}

for name, depth in evolution_depth.items():
    evolution_depth_normalized[name] = (depth - min_eval_distance )/ (max_eval_distance - min_eval_distance)

#%% md
# # Task 3)
# 
#%%
import numpy as np

sequences = np.array([str(record.seq) for record in alignment])

alignment_length = len(sequences[0])

# matrix for sequences
matrix = np.array([[s for s in seq] for seq in sequences])

#%%
order_of_sequences = [record.id for record in alignment]
output_fract = {}
score_of_conservacy = {}

# order that i will use
amino_acids_order = list("ARNDCQEGHILKMFPSTWYV")

conservation_scores_df = pd.DataFrame(
    index=amino_acids_order,
    columns=range(1, alignment_length + 1),  # Sloupce číslované od 1
)

for column_idx in range(alignment_length):
    column_data = matrix[:, column_idx]

    current_column_total_weight = 0
    for i in range(len(column_data)):
        current_column_total_weight += evolution_depth_normalized[order_of_sequences[i]]

    if current_column_total_weight == 0:
        for acid_char_loop in amino_acids_order:
            conservation_scores_df.loc[acid_char_loop, column_idx + 1] = 0.0
        continue

    for acid_char in amino_acids_order:

        current_acid_weighted_sum = 0

        for i, sequence_symbol_in_column in enumerate(column_data):
            if sequence_symbol_in_column == acid_char:
                current_acid_weighted_sum += evolution_depth_normalized[order_of_sequences[i]]

        conservation_scores_df.loc[acid_char, column_idx + 1] = current_acid_weighted_sum / current_column_total_weight

conservation_scores_df.to_csv("conservation_scores.csv", index=True)
conservation_scores_df
#%% md
# # Task 4)
# - load the aaindex file and normalization
#%%
def load_aaindex(file_path):
    """
    Simple *****code function to load aaindex data
    """
    properties = {}
    with open(file_path, 'r') as f:
        lines = [line.strip() for line in f if line.strip()]

    i = 0
    while i < len(lines):
        prop_name = lines[i]  # Name
        header_line = lines[i + 1]  # pairs
        row1 = lines[i + 2]  # fst line
        row2 = lines[i + 3]  # scd line

        pairs = header_line.split()
        values1 = [float(x) for x in row1.split()]
        values2 = [float(x) for x in row2.split()]

        prop_dict = {}
        for pair, v1, v2 in zip(pairs, values1, values2):
            aa1, aa2 = pair.split('/')
            prop_dict[aa1] = v1
            prop_dict[aa2] = v2

        properties[prop_name] = prop_dict
        i += 4
    return properties


file_path = "aaindex.txt"
aaindex_data = load_aaindex(file_path)

print(aaindex_data)

for features in aaindex_data.keys():

    # min-max normalization
    max_v = max(aaindex_data[features].values())
    min_v = min(aaindex_data[features].values())

    for k, v in aaindex_data[features].items():
        aaindex_data[features][k] = (v - min_v) / (max_v - min_v)

#%% md
# # Task 5)
#%%
import pandas as pd

physchem_property_names = list(aaindex_data.keys())

# inicalization of the output dataframe (26xN) where N is the length of the sequence
output_df = pd.DataFrame(
    index=amino_acids_order,
    columns=range(1, len(sequences[0]) + 1),  # Sloupce číslované od 1 do N
    dtype=float  # Ukládáme desetinná skóre
)


# interation over all columns of query sequence (sequences[0])
for j in range(len(sequences[0])):
    # Query (original) amino acid at position j
    aa_original = sequences[0][j]

    # -- If the query is "-" then whole column is "-"
    if aa_original == '-':
        output_df[j + 1] = '-'
        continue

    # get score of conservation for original AA (aa_original) at position j
    cons_score_original = conservation_scores_df.loc[aa_original, j + 1]

    # all amino acids:
    for aa_mutant in amino_acids_order:
        mutation_score = 0.0

        cons_score_mutant = conservation_scores_df.loc[aa_mutant, j + 1]

        # for all physical and chemical properties
        for property_name in physchem_property_names:
            property_values = aaindex_data[property_name]

            # get values for original AA
            physchem_original_p = property_values.get(aa_original, 0.0)

            # get values for mutant (new one) AA
            physchem_mutant_p = property_values.get(aa_mutant, 0.0)

            # calculate the term
            term_p = (cons_score_original * physchem_original_p) - (cons_score_mutant * physchem_mutant_p)

            # summing the term
            mutation_score += term_p


        # save a total sum of the mutation score
        output_df.loc[aa_mutant, j + 1] = mutation_score

# add column with index
output_df.insert(0, 'AA', output_df.index)

output_df.to_csv("prediction_results.csv", index=False)
output_df
