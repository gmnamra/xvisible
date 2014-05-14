#!/bin/bash

  find . -name "*.cpp" -print0 | xargs -0 perl -pi -e 's/rcUint32/uint32/g'
  find . -name "*.cpp" -print0 | xargs -0 perl -pi -e 's/rcUint16/uint16/g'
  find . -name "*.cpp" -print0 | xargs -0 perl -pi -e 's/rcUint8/uint8/g'
  find . -name "*.cpp" -print0 | xargs -0 perl -pi -e 's/rcInt32/int32/g'
  find . -name "*.cpp" -print0 | xargs -0 perl -pi -e 's/rcInt16/int16/g'
  find . -name "*.cpp" -print0 | xargs -0 perl -pi -e 's/rcInt8/int8/g'
  find . -name "*.cpp" -print0 | xargs -0 perl -pi -e 's/rcInt64/int64/g'

  find . -name "*.h*" -print0 | xargs -0 perl -pi -e 's/rcUint32/uint32/g'
  find . -name "*.h*" -print0 | xargs -0 perl -pi -e 's/rcUint16/uint16/g'
  find . -name "*.h*" -print0 | xargs -0 perl -pi -e 's/rcUint8/uint8/g'
  find . -name "*.h*" -print0 | xargs -0 perl -pi -e 's/rcInt8/int8/g'
  find . -name "*.h*" -print0 | xargs -0 perl -pi -e 's/rcInt16/int16/g'
  find . -name "*.h*" -print0 | xargs -0 perl -pi -e 's/rcInt32/int32/g'
  find . -name "*.h*" -print0 | xargs -0 perl -pi -e 's/rcInt64/int64/g'

  find . -name "*.txx" -print0 | xargs -0 perl -pi -e 's/rcUint32/uint32/g'
  find . -name "*.txx" -print0 | xargs -0 perl -pi -e 's/rcUint16/uint16/g'
  find . -name "*.txx" -print0 | xargs -0 perl -pi -e 's/rcUint8/uint8/g'
  find . -name "*.txx" -print0 | xargs -0 perl -pi -e 's/rcInt32/int32/g'
  find . -name "*.txx" -print0 | xargs -0 perl -pi -e 's/rcInt16/int16/g'
  find . -name "*.txx" -print0 | xargs -0 perl -pi -e 's/rcInt8/int8/g'
  find . -name "*.txx" -print0 | xargs -0 perl -pi -e 's/rcInt64/int64/g'

