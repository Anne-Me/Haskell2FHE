{-# LANGUAGE DataKinds         #-}
{-# LANGUAGE DeriveAnyClass    #-}
{-# LANGUAGE DeriveGeneric     #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE RecordWildCards   #-}
{-# LANGUAGE TypeApplications  #-}
{-# LANGUAGE GeneralizedNewtypeDeriving #-}
{-# LANGUAGE DerivingStrategies #-}
{-# LANGUAGE GADTs #-}
{-# LANGUAGE TypeFamilies #-}

module Example.Project (topEntity
) where

import Clash.Prelude
import GHC.Generics (Generic)


octets :: BitVector 32 -> Vec 4 (BitVector 8)
octets = bitCoerce


fromOctets :: Vec 4 (BitVector 8) -> BitVector 32
fromOctets = bitCoerce

-- Bayer-Peralta sBox
sBox :: BitVector 8 -> BitVector 8
sBox x = pack (s0, complement s1, complement s2, s3, s4, s5, complement s6, complement s7)
--inv x = pack (complement s7, complement s6, s5, s4, s3, s2, complement s1, s0)
  where
    (u0,u1,u2,u3,u4,u5,u6,u7) = unpack x :: (Bit,Bit,Bit,Bit,Bit,Bit,Bit,Bit)

    -- T stage XORs
    t1  = u0 `xor` u3
    t2  = u0 `xor` u5
    t3  = u0 `xor` u6
    t4  = u3 `xor` u5
    t5  = u4 `xor` u6
    t6  = t1 `xor` t5
    t7  = u1 `xor` u2
    t8  = u7 `xor` t6
    t9  = u7 `xor` t7
    t10 = t6 `xor` t7
    t11 = u1 `xor` u5
    t12 = u2 `xor` u5
    t13 = t3 `xor` t4
    t14 = t6 `xor` t11
    t15 = t5 `xor` t11
    t16 = t5 `xor` t12
    t17 = t9 `xor` t16
    t18 = u3 `xor` u7
    t19 = t7 `xor` t18
    t20 = t1 `xor` t19
    t21 = u6 `xor` u7
    t22 = t7 `xor` t21
    t23 = t2 `xor` t22
    t24 = t2 `xor` t10
    t25 = t20 `xor` t17
    t26 = t3 `xor` t16
    t27 = t1 `xor` t12

    -- M stage AND/XORs
    m1  = t13 .&. t6
    m2  = t23 .&. t8
    m3  = t14 `xor` m1
    m4  = t19 .&. u7
    m5  = m4 `xor` m1
    m6  = t3 .&. t16
    m7  = t22 .&. t9
    m8  = t26 `xor` m6
    m9  = t20 .&. t17
    m10 = m9 `xor` m6
    m11 = t1 .&. t15
    m12 = t4 .&. t27
    m13 = m12 `xor` m11
    m14 = t2 .&. t10
    m15 = m14 `xor` m11
    m16 = m3 `xor` m2
    m17 = m5 `xor` t24
    m18 = m8 `xor` m7
    m19 = m10 `xor` m15
    m20 = m16 `xor` m13
    m21 = m17 `xor` m15
    m22 = m18 `xor` m13
    m23 = m19 `xor` t25
    m24 = m22 `xor` m23
    m25 = m22 .&. m20
    m26 = m21 `xor` m25
    m27 = m20 `xor` m21
    m28 = m23 `xor` m25
    m29 = m28 .&. m27
    m30 = m26 .&. m24
    m31 = m20 .&. m23
    m32 = m27 .&. m31
    m33 = m27 `xor` m25
    m34 = m21 .&. m22
    m35 = m24 .&. m34
    m36 = m24 `xor` m25
    m37 = m21 `xor` m29
    m38 = m32 `xor` m33
    m39 = m23 `xor` m30
    m40 = m35 `xor` m36
    m41 = m38 `xor` m40
    m42 = m37 `xor` m39
    m43 = m37 `xor` m38
    m44 = m39 `xor` m40
    m45 = m42 `xor` m41
    m46 = m44 .&. t6
    m47 = m40 .&. t8
    m48 = m39 .&. u7
    m49 = m43 .&. t16
    m50 = m38 .&. t9
    m51 = m37 .&. t17
    m52 = m42 .&. t15
    m53 = m45 .&. t27
    m54 = m41 .&. t10
    m55 = m44 .&. t13
    m56 = m40 .&. t23
    m57 = m39 .&. t19
    m58 = m43 .&. t3
    m59 = m38 .&. t22
    m60 = m37 .&. t20
    m61 = m42 .&. t1
    m62 = m45 .&. t4
    m63 = m41 .&. t2

    -- L stage XORs
    l0  = m61 `xor` m62
    l1  = m50 `xor` m56
    l2  = m46 `xor` m48
    l3  = m47 `xor` m55
    l4  = m54 `xor` m58
    l5  = m49 `xor` m61
    l6  = m62 `xor` l5
    l7  = m46 `xor` l3
    l8  = m51 `xor` m59
    l9  = m52 `xor` m53
    l10 = m53 `xor` l4
    l11 = m60 `xor` l2
    l12 = m48 `xor` m51
    l13 = m50 `xor` l0
    l14 = m52 `xor` m61
    l15 = m55 `xor` l1
    l16 = m56 `xor` l0
    l17 = m57 `xor` l1
    l18 = m58 `xor` l8
    l19 = m63 `xor` l4
    l20 = l0 `xor` l1
    l21 = l1 `xor` l7
    l22 = l3 `xor` l12
    l23 = l18 `xor` l2
    l24 = l15 `xor` l9
    l25 = l6 `xor` l10
    l26 = l7 `xor` l9
    l27 = l8 `xor` l10
    l28 = l11 `xor` l14
    l29 = l11 `xor` l17

    -- Final output bits
    s0 = l6 `xor` l24
    s1 = l16 `xor` l26
    s2 = l19 `xor` l28
    s3 = l6 `xor` l21
    s4 = l20 `xor` l22
    s5 = l25 `xor` l29
    s6 = l13 `xor` l27
    s7 = l6 `xor` l23


-- | Apply S-box to each of the four bytes of a 32-bit word
subWord :: BitVector 32 -> BitVector 32
subWord w = fromOctets (map sBox (octets w))

-- | Apply SubBytes to your AES State (leaving the round-key schedule untouched)
subBytes :: Vec 4 (BitVector 32)  -> Vec 4 (BitVector 32) 
subBytes  ws = map subWord ws

-- | Rotate a Vec 4 left by 1 slot
rotateLeft1 :: Vec 4 a -> Vec 4 a
rotateLeft1 (x :> xs) = xs :< x

-- | Rotate a Vec 4 left by k slots (k in [0..3])
rotateLeftVec :: Int -> Vec 4 a -> Vec 4 a
rotateLeftVec 0 v = v
rotateLeftVec k v = rotateLeftVec (k - 1) (rotateLeft1 v)

-- | Rotate a length-4 Vec left by 0..3 positions.
rotateLeft4 :: Int -> Vec 4 a -> Vec 4 a
rotateLeft4 0 v                  = v
rotateLeft4 1 (a :> b :> c :> d :> Nil) = b :> c :> d :> a :> Nil
rotateLeft4 2 (a :> b :> c :> d :> Nil) = c :> d :> a :> b :> Nil
rotateLeft4 3 (a :> b :> c :> d :> Nil) = d :> a :> b :> c :> Nil
rotateLeft4 _ v                  = v  -- fallback, should never happen

-- | AES ShiftRows step: row 0 by 0, row 1 by 1, row 2 by 2, row 3 by 3 bytes.
shiftRows :: Vec 4 (BitVector 32)  -> Vec 4 (BitVector 32) 
shiftRows ws =
  let cols = map octets ws
      rows = transpose cols
      shifted = zipWith rotateLeft4 (iterateI (+1) 0) rows
      newCols = transpose shifted
  in map fromOctets newCols

-- double in GF(2⁸): xtime(x)
gfMul2 :: BitVector 8 -> BitVector 8
gfMul2 b = pack $ zipWith xor (tail bits :< False) (unpack (if msb then 0x1B else 0))
  where
    bits@(msb :> _rest) = unpack b :: Vec 8 Bool

-- multiply by 3 = xtime(x) ⊕ x
gfMul3 :: BitVector 8 -> BitVector 8
gfMul3 b = gfMul2 b `xor` b

-- | MixColumn coefficient [2 3 1 1] ⋅ [s0 s1 s2 s3]^T
mixCol0
  :: BitVector 8 -> BitVector 8 -> BitVector 8 -> BitVector 8 -> BitVector 8
mixCol0 s0 s1 s2 s3 = gfMul2 s0 `xor` gfMul3 s1 `xor` s2 `xor` s3

-- | MixColumn coefficient [1 2 3 1]
mixCol1
  :: BitVector 8 -> BitVector 8 -> BitVector 8 -> BitVector 8 -> BitVector 8
mixCol1 s0 s1 s2 s3 = s0 `xor` gfMul2 s1 `xor` gfMul3 s2 `xor` s3

-- | MixColumn coefficient [1 1 2 3]
mixCol2
  :: BitVector 8 -> BitVector 8 -> BitVector 8 -> BitVector 8 -> BitVector 8
mixCol2 s0 s1 s2 s3 = s0 `xor` s1 `xor` gfMul2 s2 `xor` gfMul3 s3


-- | MixColumn coefficient [3 1 1 2]
mixCol3
  :: BitVector 8 -> BitVector 8 -> BitVector 8 -> BitVector 8 -> BitVector 8
mixCol3 s0 s1 s2 s3 = gfMul3 s0 `xor` s1 `xor` s2 `xor` gfMul2 s3


-- | AES MixColumns step: apply the column mix to each of the 4 words in the state
mixColumns :: Vec 4 (BitVector 32)  -> Vec 4 (BitVector 32) 
mixColumns ws = map mixWord ws
  where
    mixWord w =
      let b0 :> b1 :> b2 :> b3 :> Nil = octets w
          m0 = mixCol0 b0 b1 b2 b3
          m1 = mixCol1 b0 b1 b2 b3
          m2 = mixCol2 b0 b1 b2 b3
          m3 = mixCol3 b0 b1 b2 b3
      in fromOctets (m0 :> m1 :> m2 :> m3 :> Nil)

-- | Rotate a 32-bit word by one byte to the left:
--   [b0,b1,b2,b3] → [b1,b2,b3,b0]
rotWord :: BitVector 32 -> BitVector 32
rotWord w =
  let
    -- break into 4 bytes
    b0 :> b1 :> b2 :> b3 :> Nil = octets w
    -- rotate left by one byte
    rotated = b1 :> b2 :> b3 :> b0 :> Nil
  in
    -- reassemble back into a 32-bit word
    fromOctets rotated


-- | XOR in one round-key (4 words) into the State
addRoundKeyWith
  :: BitVector 128
  -> BitVector 128
  -> BitVector 128
addRoundKeyWith a b = a `xor` b

-- | AES-128 single-block encrypt
aesBlockEncrypt
  :: BitVector 128   -- 16 bytes
  -> Vec 11 (BitVector 128)         -- 44 bytes
  -> BitVector 128  -- 16 bytes
aesBlockEncrypt pt allKs = finalState
--finalState
  where
    -- initial AddRoundKey (round 0)
    state0 = addRoundKeyWith pt (allKs !! 0)

    --rounds 1..9: SubBytes → ShiftRows → MixColumns → AddRoundKey
    midState :: BitVector 128
    midState = foldl
      (\st rk -> addRoundKeyWith (bitCoerce (mixColumns (shiftRows (subBytes (bitCoerce st))))) rk)
      state0
      (init (tail allKs))

    -- final round (round 10): SubBytes → ShiftRows → AddRoundKey
    finalState = addRoundKeyWith (bitCoerce (shiftRows (subBytes (bitCoerce midState))))
      (allKs !! 10)


-- | 'topEntity' is Clash's equivalent of 'main' in other programming
-- languages. Clash will look for it when compiling 'Example.Project'
-- and translate it to HDL. While polymorphism can be used freely in
-- Clash projects, a 'topEntity' must be monomorphic and must use non-
-- recursive types. Or, to put it hand-wavily, a 'topEntity' must be
-- translatable to a static number of wires.
topEntity :: BitVector 128 -> Vec 11 (BitVector 128) -> BitVector 128
topEntity x y = aesBlockEncrypt x y
