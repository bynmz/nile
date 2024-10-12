function fib(n)
    if n == 1 or n == 2 then
	    return 1,1
	end
	prev, prevprev = fib(n-1)
	return prev+prevprev, prev
end

function add(a,b)
	return a+b
end

	    
sum = function(...)
   local result = 0
   local arg = { ... }
   for i,v in ipairs(arg) do
      result = result + v
   end
   return result
end
  



print(fib(5))
print((fib(10)))


