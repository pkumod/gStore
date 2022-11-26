def ResultAllSameChecker(correct_results, output_results):
    correct_results.sort()
    output_results.sort()
    return correct_results == output_results


def ResultLimitKChecker(correct_results, output_results, k):
    correct_len = len(correct_results)
    output_len = len(output_results)
    if correct_len >= k and output_len != k:
        return False
    ia, ib = 0, 0
    correct_results.sort()
    output_results.sort()
    while ia < correct_len and ib < output_len:
        if correct_results[ia] == correct_results[ib]:
            ia += 1
            ib += 1
        elif correct_results[ia] < correct_results[ib]:
            ia += 1
        else: # correct_results[ia] > correct_results[ib]:
            return False
    return ib == output_len
