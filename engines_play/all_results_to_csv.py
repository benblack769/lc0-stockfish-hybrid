from compile_results import get_results
from collections import defaultdict
import os
import sys

def all_results_to_table(all_folders):
    result_table = {}
    for fold in all_folders:
        result_dict = get_results(fold)
        engine_names = sorted(set(result_dict.keys()) - {'drawn'})
        print(result_dict)

def fold_results_to_table(games_folder):
    folders = []
    for item in os.listdir(games_folder):
        path = os.path.join(games_folder, item)
        folders.append(path)
    return all_results_to_table(folders)

def table_to_csv(table):
    file_data = "\n".join([','.join(row) for row in table]) + '\n'
    return file_data
    # with open(path, 'w') as file:
    #     file.write(file_data)

if __name__ == "__main__":
    assert len(sys.argv) == 2

    folder = sys.argv[1]
    benchmark_table = fold_results_to_table(folder)
    csv_data = table_to_csv(benchmark_table)
    print(csv_data)
