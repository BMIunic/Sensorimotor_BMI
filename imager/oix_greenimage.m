function out = oix_greenimage(V) 
% function oix_greenimage(V) 
% function label = oix_greenimage('label') 

if ischar(V) && strcmp(V,'label')
    out = 'Green Image';
    return
end

% Get the tpv_content object
C = V.content;

% Get file name and read picture
if isfield(C.user,'greenimage') && ~isempty(C.user.greenimage) && isnumeric(C.user.greenimage)
    a = C.user.greenimage;
else
    fname = fn_getfile({'*.bmp;*.BMP;*.tif;*.tiff'},'Select green image');
    if ~fname, return, end % canceled
    a = fn_readimg(fname);
end
bin = floor(size(a)./C.trial.sizes(1:2));
if diff(bin) || any(size(fn_bin(a,bin))~=C.trial.sizes(1:2)), errordlg 'size does not match data', return, end
bin = bin(1);
C.user.greenimage = a;
%if any(bin>1), a = fn_bin(a,bin); end


% Display or return image
if nargout==0
    % display
    scale = [V.dx/bin V.dy/bin V.dt];
    mat = {scale -scale/2};
    R = rotation(V.a4d.F,'mat',mat);
    SI = projection(R,[1 2],'data',a);
    activedisplayImage(SI,'in',1);
else
    % return image
    out = a;
end