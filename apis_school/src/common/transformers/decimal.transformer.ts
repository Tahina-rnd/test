import { ValueTransformer } from 'typeorm';
import { Decimal } from 'decimal.js';

/**
 * Transformer pour convertir Decimal.js ↔ string en base de données
 * Utilisé avec les colonnes `decimal` de TypeORM
 */
export const DecimalTransformer: ValueTransformer = {
  to: (value: Decimal | null | undefined): string => {
    return value?.toString() ?? '0';
  },
  from: (value: string | null | undefined): Decimal => {
    if (!value) return new Decimal(0);
    try {
      return new Decimal(value);
    } catch (error) {
      console.warn(`Invalid decimal value from DB: ${value}`);
      return new Decimal(0);
    }
  },
};
