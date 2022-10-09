PROGRAM STATS
VAR
    SUM,SUMSQ,I,VALUE,MEAN,VARIANCE,I,HEY  :   INTEGER;
    VARIANCE : REAL
BEGIN
    HEY := 0.1;
    SUM :=  0+Z;
    FOR I   :=  1   TO  100.0 DO
        BEGIN
            READ(VALUE);
            SUM :=  SUM +   VALUE;
            SUMSQ   :=  SUMSQ   +   VALUE*VALUE
        END;
    $    
    MEAN  :=  SUM DIV 100.0;
    VARIANCE    :=  SUMSQ   DIV 100 -   MEAN*MEAN;
    WRITE(MEANY,VARIANCE)
    XXXX
END.