
function _ready()
{
    var sub = (a, b) => a - b;
    var add = (a, b) => a + b;
    var mul = (a, b) => a * b;

    var oprts = [sub, add, mul];

    console.log(oprts[1](10, 5));
    console.log(mul(add(10, 5), sub(5, 10)));
    var mi = ((n, nmin) => { if ( n < nmin) n = nmin; return n })
    var ma = ((n, nmax) => {if ( n > nmax) n = nmax; return n})
    console.log(mi(ma(5, 10), 0))
    console.log(((n, nmin) => { if ( n < nmin) n = nmin; return n })(((n, nmax) => {if ( n > nmax) n = nmax; return n})(5, 10), 0));
}