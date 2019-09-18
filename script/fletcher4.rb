def weak_mod(a)
  (a >> 16) + (a & 0xFFFF)
end

nums=[weak_mod(2**32-1)]*4
loop.with_index(0) do |_, ix|
  input=0xffff
  nums[0]=+input
  nums[1]+=nums[0]
  nums[2]+=nums[1]
  nums[3]+=nums[2]
  if nums.any?{ |n| 2**31<=n }
    p ix
    break
  end
end
