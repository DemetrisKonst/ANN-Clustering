# ANN Analysis <br>
In this file, the ANN algorithms' results will be analyzed, depending on the parameters given.
<br>
### Locality Sensitive Hashing
Keep in mind that these algorithms have an element of randomness, so results may differ among executions (with the same parameters)
The parameters that can be modified are the following:
- W (window size)
- L (number of hash tables)
- k (number of hash functions that create amplified hash function)
##### Window Size (W)
Let us assume constant values for L and k. In these examples L = 7 and k = 5.
1. W = 40000 <br>
    - Time Ratio (average time for LSH kNN / average time for brute force kNN) : 0.6182
    - Accuracy (mean distance for brute force kNN / mean distance for LSH kNN): 99.394% <br>
    We can safely assume that LSH with those parameters is extremely accurate. However, the time consumed to query the items does not differ much from the brute force implementation.
1. W = 10000 <br>
    - Time Ratio: 0.1265
    - Accuracy: 72.537% <br>
    When W equals 10000, we notice significant change from 40000. The time ratio has dropped to 0.12 (from 0.6) and the accuracy has dropped to 72% (from 99%)
1. W = 4000  <br>
    - Time Ratio: 0.1256
    - Accuracy: 56.105% <br>
    With W = 4000 although the algorithm may seem fine timewise (almost 1/10 of brute force and a bit lower from W=10000), the accuracy drops dramatically to nearly 56%.
1. W = Average Item Distance * Constant. <br>
    Through brute force, the average item distance of the items is calculated (for a portion of the training set). This distance is then multiplied by a constant C.
    Average Item Distance is approximately 33000 (calculated from 5% of the training set)
    1. C = 1 (Window Size = 33184)
        - Time Ratio: 0.5133
        - Accuracy: 99.122% <br>
        This execution is similar to W = 40000. Accuracy drops by a little, but time ratio drops by 0.10
    2. C = 2 (Window Size = 66369)
        - Time Ratio: 2.4336
        - Accuracy: 99.989% <br>
        With a window size of 66369 we notice that LSH is more time consuming than brute force! Accuracy is extremely high, however this implementation is utterly useless.
<br>
There is always a trade-off between accuracy and speed. The best values for W (as examined above) are 40000, 10000 and Average Item Distance * 1.
These values achieve a balanced trade-off and it is up to the user to select which fits his model best. <br>
 
##### Number of Hash Tables (L)
Let us assume constant values for W and k. In these examples k = 5 and W = Average Item Distance * 1 (~=33000)
1. L = 1
    - Time Ratio: 0.1161
    - Accuracy: 90.228% <br>
    Almost 1/10 faster than brute force, with an acceptable accuracy.
2. L = 2
    - Time Ratio: 0.1466
    - Accuracy: 93.383% <br>
    A little bit slower than the previous example, accuracy has improved by almost 3%
3. L = 4
    - Time Ratio: 0.1562
    - Accuracy: 95.006% <br>
    Very small time difference than L=2, yet accuracy has improved by almost 2%
4. L = 6
    - Time Ratio: 0.3575
    - Accuracy: 98.270 <br>
    Time has almost doubled. Accuracy has improved by more than 3%
5. L = 8
    - Time Ratio: 0.8860
    - Accuracy: 99.621% <br>
    This value of L seems to be a slight "overkill" as speed is close to brute force. Accuracy, however, is at a very respectable 99.621%
<br>
All values of L presented above are respectable in terms of balanced accuracy vs performance. 
For calculating the best value of k we will use L = 4 as it is overall fast and also highly accurate. <br>

##### Number of Hash Functions in Amplified Hash Function (k)
Let us assume constant values for L and W. In the following examples L = 4 and W = Average Item Distance * 1 (~=33000)
1. k = 2
    - Time Ratio: 1.0734
    - Accuracy: 99.851% <br>
    Performance is worse than brute force, accuracy is close to 100%. Not acceptable.
2. k = 3
    - Time Ratio: 0.4442
    - Accuracy: 98.165 <br>
    Performance is a lot better than k = 2 and accuracy is less than 2% lower.
3. k = 4
    - Time Ratio: 0.2814
    - Accuracy: 96.648 <br>
    Performance is almost half of k = 3, accuracy has dropped almost the same amount.
4. k = 5
    - Time Ratio: 0.2959
    - Accuracy: 96.736 <br>
    Performance and accuracy are almost the same as in k = 4.
5. k = 6
    - Time Ratio: 0.3851
    - Accuracy: 98.353 <br>
    Performance and accuracy are near to the results of k = 3 indicating that there is no linear correlation between k and accuracy or performance. <br>
    
##### Summary
Disregarding some edge cases, we can clearly see that the user can parameterize LSH to the way they see fit. For example a really fast
combination could be W = 10000, k = 4, L = 1 with Time Ratio = 0.01 (!) yet its accuracy is at 47%. On the contrary, a very accurate
combination (without exceeding brute force times) is W = Average Item Distance * 1, k = 5, L = 7 with a time ratio of 0.88 (close to brute force)
but accuracy of 99.621%. 
