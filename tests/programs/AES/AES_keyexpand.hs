{-# LANGUAGE DataKinds         #-}
{-# LANGUAGE DeriveAnyClass    #-}
{-# LANGUAGE DeriveGeneric     #-}
{-# LANGUAGE NoImplicitPrelude #-}
{-# LANGUAGE RecordWildCards   #-}
{-# LANGUAGE TypeApplications  #-}
{-# LANGUAGE GeneralizedNewtypeDeriving #-}

module AES.KeyExpansion (topEntity
  , nistKeyExpand
  , KeySchedule(..)
  , Key(..)
  , Plaintext(..)
  , Ciphertext(..)
) where

import Clash.Prelude
import GHC.Generics (Generic)

-- AESencrypt :: BitVector 128 -> BitVector 128 -> BitVector 128
-- AESencrypt key plaintext = cipher


expandPolynomial :: BitVector 32 -> Vec 32 (BitVector 32)
expandPolynomial x = map (.&. x) masks
  where
    -- masks = [1,2,4,8,…,2^31] as a statically‐sized vector
    masks :: Vec 32 (BitVector 32)
    masks = iterateI (`shiftL` 1) 1

polyMult  :: BitVector 32  -> BitVector 32 -> BitVector 32 
polyMult x y = foldl xor 0 (map (\m -> x * m) (expandPolynomial y))

idxsDescInt :: Vec 32 Int
idxsDescInt = reverse (iterateI (+1) (0 :: Int))

highestIndex  :: BitVector 32  -> Index 32
highestIndex x = foldr go (0 :: Index 32) idxsDescInt
  where
    go :: Int -> Index 32 -> Index 32
    go i acc = if testBit x i then fromIntegral i else acc

polyRemainder  :: BitVector 32 -> BitVector 32  -> BitVector 32  -- ^ remainder
polyRemainder x y = foldl step x idxsDescInt
  where
    degYInt :: Int
    degYInt = fromIntegral (highestIndex y)

    step :: BitVector 32 -> Int -> BitVector 32
    step acc i
      -- if bit i is set and we're at or above y's degree, xor-shift y
      | testBit acc i && i >= degYInt
      = acc `xor` (y `shiftL` (i - degYInt))
      | otherwise
      = acc

gfMult :: BitVector 32 -> BitVector 32 -> BitVector 32
gfMult x y = polyRemainder (polyMult x y) 0x11B

-- | GF(2^8) multiply two bytes via your 32-bit gfMult + slice back to 8 bits
gfMult8 :: BitVector 8 -> BitVector 8 -> BitVector 8
gfMult8 x y = slice d7 d0 $ gfMult (zeroExtend x) (zeroExtend y)


octets :: BitVector 32 -> Vec 4 (BitVector 8)
octets bv =
      slice d31 d24 bv :>
      slice d23 d16 bv :>
      slice d15 d8  bv :>
      slice d7  d0  bv :>
      Nil


fromOctets :: Vec 4 (BitVector 8) -> BitVector 32
fromOctets (b3 :> b2 :> b1 :> b0 :> Nil) =
      (zeroExtend b3 `shiftL` 24)
  .|. (zeroExtend b2 `shiftL` 16)
  .|. (zeroExtend b1 `shiftL` 8)
  .|.  zeroExtend b0

type BlockSize = 16

-- | 16-byte plaintext, as a statically-sized vector of bytes
newtype Plaintext = Plaintext (Vec BlockSize (BitVector 8))
  deriving stock   (Generic)
  deriving newtype (Eq, Show, NFDataX)

-- | 16-byte ciphertext, same shape
newtype Ciphertext = Ciphertext (Vec BlockSize (BitVector 8))
  deriving stock   (Generic)
  deriving newtype (Eq, Show, NFDataX)


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

-- | Apply S-box to each of the four bytes of a 32-bit word
subWord :: BitVector 32 -> BitVector 32
subWord w = fromOctets (map subByte (octets w))

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

-- | AES round‐constant words for AES-128 (10 entries),
--   each = [ rcon_i, 0, 0, 0 ] as a 32‐bit word.
rcon :: Vec 10 (BitVector 32)
rcon = map toWord32 (iterateI (gfMult8 0x02) 0x01)
  where
    toWord32 :: BitVector 8 -> BitVector 32
    toWord32 w = fromOctets (w :> 0 :> 0 :> 0 :> Nil)


-- | Expand a 4-word Key into the full 44-word KeySchedule
keyExpand
  :: (Vec 4 (BitVector 32))
  -> ( (Unsigned 8)               -- ^ Nk = 4
     -> (Unsigned 8)                  -- ^ current index i (0–43)
     -> (BitVector 32)                 -- ^ previous k_i
     -> BitVector 32            -- ^ previous temp
     -> BitVector 32            -- ^ new temp
     )
  -> (Vec 44 (BitVector 32))
keyExpand keyWords subFunc =
  schedule
 where
  -- A Vec of all indices 0..43
  idxs :: Vec 44 (Index 44)
  idxs = iterateI (+1) (0 :: Index 44)

  -- Build each word by mapping our recurrence over those indices
  schedule :: Vec 44 (BitVector 32)
  schedule = map go idxs

  go :: Index 44 -> BitVector 32
  go i
    -- i < Nk? Then pull from the original keyWords
    | i < fromIntegral (natToNum  4)
    = keyWords !! fromIntegral i

    -- otherwise, AES-128 recurrence:
    | otherwise
    = let prev   = schedule !! (i - 1)
          prevNk = schedule !! (i -  4)
          idx    = fromIntegral i
          kWord  = prevNk
          temp1  = subFunc (4)
                           idx
                           kWord
                           prev
      in  temp1 `xor` prevNk

-- | AES‐128 key‐schedule “SubWord/RotWord” step
subFuncAes128
  :: (Unsigned 8)          -- Nk (4)
  -> (Unsigned 8)            -- current index i
  -> (BitVector 32)            -- previous k_i
  -> BitVector 32       -- previous temp_i
  -> BitVector 32       -- next temp_{i+1}
subFuncAes128 kl  i  k temp
  | i `mod` kl == 0 = subWord (rotWord temp) `xor` (rcon !! fromIntegral (i `div` kl - 1))
  | otherwise       = temp


nistKeyExpand :: (Vec 4 (BitVector 32)) -> (Vec 44 (BitVector 32))
nistKeyExpand raw = keyExpand raw subFuncAes128


-- | 'topEntity' is Clash's equivalent of 'main' in other programming
-- languages. Clash will look for it when compiling 'Example.Project'
-- and translate it to HDL. While polymorphism can be used freely in
-- Clash projects, a 'topEntity' must be monomorphic and must use non-
-- recursive types. Or, to put it hand-wavily, a 'topEntity' must be
-- translatable to a static number of wires.
topEntity :: (Vec 4 (BitVector 32)) -> KeySchedule
topEntity key = nistKeyExpand key
