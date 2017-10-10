class Solution(object):
    def findLongestChain(self, pairs):
        """
        :type pairs: List[List[int]]
        :rtype: int
        """
        if(len(pairs)==0):
        	return 0
        if(len(pairs)==1):
        	return 1

        pairs.sort(key=lambda x:x[0])

       	dp=[1]*len(pairs)
       	
       	for i in range(1,len(pairs)):
       		for j in range(i-1,-1,-1):
       			if(pairs[i][0]>pairs[j][1]):
       				dp[i]=dp[j]+1
       				break
       				
       	length=0
       	for i in dp:
       		if (i>length):
       			length=i
       	return length


print(Solution().findLongestChain([[1,2], [2,3], [3,4],[4,6],[7,10]]))



        
