require "prime"

def find_seed
  loop do
    c=([0]*8+[1]*8).shuffle.join.to_i(2)
    return c if c.prime?
  end
end

r={}
while r.size<6 do
  r[find_seed]=true
end

p r.keys
