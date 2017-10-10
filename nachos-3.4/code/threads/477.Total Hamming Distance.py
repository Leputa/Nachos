# class Solution(object):
#     def totalHammingDistance(self, nums):
#         """
#         :type nums: List[int]
#         :rtype: int
#         """
#         binary=[]
#         for i in nums:
#         	tmpB=[]
#         	while i!=0:
#         		tmpB.append(i%2)
#         		i=i//2
#         	binary.append(tmpB)
#         dis=0
#         for i in range(len(binary)-1):
#         	for j in range(i+1,len(binary)):
#         		x=0
#         		y=0
#         		while(x<len(binary[i]) and y<len(binary[j])):
#         			if(binary[i][x]!=binary[j][y]):
#         				dis+=1
#         			x+=1
#         			y+=1
#         		while(x<len(binary[i])):
#         			if(binary[i][x]==1):
#         				dis+=1
#         			x+=1
#         		while(y<len(binary[j])):
#         			if(binary[j][y]==1):
#         				dis+=1
#         			y+=1
#         return dis

class Solution(object):
    def totalHammingDistance(self, nums):
        """
        :type nums: List[int]
        :rtype

        """
        ans=0
        length=len(nums)
        for i in range(32):
        	mask=1<<i  #1 2 4 8....
        	one=0
        	for num in nums:
        		if num & mask:
        			one+=1
        	ans+=one*(len(nums)-one)
        return ans




print(Solution().totalHammingDistance([4,14,2]))


