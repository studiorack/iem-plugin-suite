xRange = -3:3;
yRange = -3:3;
zRange = -2:2;

cmbs = allcomb(xRange,yRange,zRange);
order = sum(abs(cmbs),2);
[order,idx] = sort(order);
cmbs = cmbs(idx,:);

sum(order <= 7)
refl = [cmbs(order<=7,:) order(order<=7)];
%%
mat2header('reflections',refl,1);