clear;

AR = audioDeviceReader();
AR.Device = "Soundflower (2ch)";
AR.SampleRate = 44100;
AR.NumChannels = 1;
AR.BitDepth = '32-bit float';
%%

data = [];
tic 


tic 
while toc < 10
    data = [data; step(AR)];
end

figure, plot(data);

%%
f= 4;
stftParams = struct('fs',44100,'wsz',1024*f,'hop',512*f,'fftsz',4096*f);
[S,St,Params] = stft_thirdoctave(data,stftParams);

[EDR] = edrCalc(St);
EDR = permute(EDR,[2 1 3]); % frames x bands x channels
[a,b,c,t] = edrNoisyParamFinder(EDR,1); % 1 plot on
%t = repmat((0:size(EDR,1)-1)',1,size(EDR,2),size(EDR,3));

T60 = 3*log(10).*1./b*512*f/44100; % T60 from slope of EDR
%%

plot(data);
audiowrite('ir.wav',data, AR.SampleRate);

%%

load tDesign5200
%%
W = data(:,1);
%%
decoder = getSH(7,[tDesign5200.azimuth tDesign5200.zenith],'real');
lspGains = decoder*data';
lspGainsSquared = lspGains.^2;

%%
iVec = [tDesign5200.x tDesign5200.y tDesign5200.z]'*lspGainsSquared./sum(lspGainsSquared,1);
D = sqrt(2) * sqrt(iVec(1,:).^2 + iVec(2,:).^2 + iVec(3,:).^2);

