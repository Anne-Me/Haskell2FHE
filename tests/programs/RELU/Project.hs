module Example.Project (topEntity, relu) where

import Clash.Prelude

-- | ReLU function: max(0, x). Example:
--
-- >>> relu (4)
-- 0
-- >>> relu 10
-- 10
relu :: Signed 8 -> Signed 8
relu x = if x < 5 then 0 else x

-- | 'topEntity' is Clash's equivalent of 'main' in other programming
-- languages. Clash will look for it when compiling 'Example.Project'
-- and translate it to HDL.
topEntity :: Signed 8 -> Signed 8
topEntity = relu

