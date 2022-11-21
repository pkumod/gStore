
def ResultAllSameChecker(correct_results, output_results):
    correct_results.sort()
    output_results.sort()
    return correct_results == output_results